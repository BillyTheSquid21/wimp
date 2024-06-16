#include <wimp_reciever.h>
#include <wimp_log.h>
#include <stdlib.h>

/*
* Represents the state the reciever is in
*/
enum RecieverState
{
	REC_IDLE,
	REC_READING_HEADERS,
	REC_READING_DATA,
	//TODO - add other states
};

/*
* Reciever loop
* 
* @param args The arguments to pass to the reciever
*/
void wimp_reciever_recieve(RecieverArgs args);

/*
* Allocates the instruction for the incoming queue
*/
WimpInstr wimp_reciever_allocateinstr(pssize size);

/*
* Initializes the sockets for the reciever and checks the handshake
* 
* @param recsock Pointer to the reciever socket
* @param rec_address Pointer to the reciever address
* @param args Reciever args
* 
* @return Returns either WIMP_RECIEVER_SUCCESS or WIMP_RECIEVER_FAIL
*/
int32_t wimp_reciever_init(PSocket** recsock, PSocketAddress** rec_address, RecieverArgs args);

/*
* Sets the reciever process priority
*
* @param priority The puthread thread priority
*/
void wimp_reciever_set_process_prio(enum PUThreadPriority_ priority);

int32_t wimp_get_instr_size(uint8_t* buffer)
{
	return *(int32_t*)buffer;
}

WimpHandshakeHeader wimp_create_handshake(const char* process_name, uint8_t* message_buffer)
{
	int32_t process_name_bytes = (int32_t)(strlen(process_name) + 1) * sizeof(char);
	
	WimpHandshakeHeader header = { 0, 0 }; //Values if below fails

	//Copy the header and the name
	size_t offset = sizeof(WimpHandshakeHeader);

	if (offset + process_name_bytes < WIMP_MESSAGE_BUFFER_BYTES)
	{
		memcpy(&message_buffer[offset], process_name, process_name_bytes);
		header.handshake_header = WIMP_RECIEVER_HANDSHAKE;
		header.process_name_bytes = process_name_bytes;

		memcpy(message_buffer, &header, sizeof(WimpHandshakeHeader));
	}
	return header;
}

RecieverArgs wimp_get_reciever_args(const char* process_name, const char* recfrom_domain, int32_t recfrom_port, WimpInstrQueue* incomingq, int32_t* active)
{
	RecieverArgs recargs = malloc(sizeof(struct _RecieverArgs));
	if (recargs == NULL)
	{
		return NULL;
	}

	recargs->process_name = sdsnew(process_name);
	recargs->recfrom_domain = sdsnew(recfrom_domain);
	recargs->incoming_queue = incomingq;
	recargs->recfrom_port = recfrom_port;
	recargs->active = active;
	return recargs;
}

void wimp_free_reciever_args(RecieverArgs args)
{
	sdsfree(args->process_name);
	sdsfree(args->recfrom_domain);
	free(args);
}

int32_t wimp_reciever_init(PSocket** recsock, PSocketAddress** rec_address, RecieverArgs args)
{
	WimpMsgBuffer recbuffer;
	WimpMsgBuffer sendbuffer;
	WIMP_ZERO_BUFFER(recbuffer); WIMP_ZERO_BUFFER(sendbuffer);
	PError* err;

	//Create client socket, connect to recfrom server
	//Then send handshake and process name
	WimpHandshakeHeader header = wimp_create_handshake(args->process_name, sendbuffer);

	//Construct address for client, which should be listening
    *rec_address = p_socket_address_new(args->recfrom_domain, args->recfrom_port);
    if (*rec_address == NULL)
    {
		WIMP_ZERO_BUFFER(sendbuffer);
        return WIMP_RECIEVER_FAIL;
    }

    //Create the main listen/recieve socket - currently hard coded
    *recsock = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, &err);
    if (*recsock == NULL)
    {
		wimp_log_fail("Failed to create reciever socket! (%d): %s\n", p_error_get_code(err), p_error_get_message(err));
		p_error_free(err);
		WIMP_ZERO_BUFFER(sendbuffer);
        return WIMP_RECIEVER_FAIL;
    }

    //Connect to end process, which should be waiting to accept

	//Try up to WIMP_REC_TRY_COUNT times, with set interval
	//This is because there is no timeout on the connect call
	int32_t num_tries = 0;
	bool con_success = false;
	while (num_tries < WIMP_REC_TRY_COUNT)
	{
		con_success = p_socket_connect(*recsock, *rec_address, &err);
		if (con_success)
		{
			break;
		}

		//If failed, try again
		wimp_log_important("%s reciever failed to connect - trying again...\n", args->process_name);
		num_tries++;
		p_uthread_sleep(WIMP_REC_TRY_INTERVAL);
	}

	if (!con_success)
    {
		pint code = p_error_get_code(err);
		wimp_log_fail("%s reciever failed to connect (%d)- expected connection at %s:%d\n", args->process_name, code, args->recfrom_domain, args->recfrom_port);
        p_socket_address_free(*rec_address);
        p_socket_free(*recsock);
		WIMP_ZERO_BUFFER(sendbuffer);
        return WIMP_RECIEVER_FAIL;
    }

	wimp_log_success("%s reciever connection at %s:%d\n", args->process_name, args->recfrom_domain, args->recfrom_port);

	//Send the handshake
	p_socket_send(*recsock, sendbuffer, sizeof(WimpHandshakeHeader) + header.process_name_bytes, NULL);
	WIMP_ZERO_BUFFER(sendbuffer);

	//Read next handshake
	pssize handshake_size = p_socket_receive(*recsock, recbuffer, WIMP_MESSAGE_BUFFER_BYTES, NULL);

	if (handshake_size <= 0)
	{
		wimp_log_fail("Reciever handshake failed! (%d): %s\n", p_error_get_code(err), p_error_get_message(err));
		p_error_free(err);
        p_socket_address_free(*rec_address);
        p_socket_free(*recsock);
		WIMP_ZERO_BUFFER(recbuffer);
        return WIMP_RECIEVER_FAIL;
	}

	//Check start of handshake
	WimpHandshakeHeader* recheader = (WimpHandshakeHeader*)recbuffer;
	if (recheader->handshake_header != WIMP_RECIEVER_HANDSHAKE)
	{
		wimp_log_fail("Reciever recieved invalid handshake!: %d\n", recheader->handshake_header);
        p_socket_address_free(*rec_address);
        p_socket_free(*recsock);
		WIMP_ZERO_BUFFER(recbuffer);
		return WIMP_RECIEVER_FAIL;
	}
	WIMP_ZERO_BUFFER(recbuffer);
	return WIMP_RECIEVER_SUCCESS;
}

typedef struct _WimpRecieverState
{
	//Size of the last packet to be received
	pssize incoming_size;

	//Current offset within the recievebuffer
	size_t rec_offset;

	//Current state of the reciever
	int32_t state;

	//Current instruction of the reciever
	WimpInstr instruction;

	size_t instruction_bytes_read;
} WimpRecieverState;

/*
* Gets the next packet and resets location in recbuffer
*/
void wimp_reciever_next_packet(WimpRecieverState* state, PSocket* recsock, uint8_t* recbuffer)
{
	WIMP_ZERO_BUFFER(recbuffer);
	state->incoming_size = p_socket_receive(recsock, recbuffer, WIMP_MESSAGE_BUFFER_BYTES, NULL);
	state->rec_offset = 0;
}

void wimp_reciever_recieve(RecieverArgs args)
{	
	WimpMsgBuffer recbuffer;
	WimpMsgBuffer sendbuffer;
	WIMP_ZERO_BUFFER(recbuffer); WIMP_ZERO_BUFFER(sendbuffer);

	//Initialize the sockets for the reciever and send handshake
	PSocket* recsock;
    PSocketAddress* rec_address;
	if (wimp_reciever_init(&recsock, &rec_address, args) == WIMP_RECIEVER_FAIL)
	{
		wimp_free_reciever_args(args);
		p_uthread_exit(WIMP_RECIEVER_FAIL);
		return;
	}

	//State of the reciever
	WimpRecieverState state = 
	{ 
		0, 
		0, 
		REC_IDLE,
		{ NULL, 0 },
		0
	};

	bool disconnect = false;
	while (!disconnect)
	{
		/*
		* CHECK PROCESS: checks if the process is still active
		*/
		if (!p_atomic_int_get(args->active))
		{
			//If an instruction being built, clear it
			if (state.instruction.instruction)
			{
				free(state.instruction.instruction);
			}
			break;
		}

		/*
		* IDLE STATE: continuously poll for a new packet until one is recieved
		* If a packet is recieved, enter reading headers mode
		*/
		if (state.state == REC_IDLE)
		{
			wimp_reciever_next_packet(&state, recsock, recbuffer);
			if (state.incoming_size > 0)
			{
				state.state = REC_READING_HEADERS;
			}
		}

		/*
		* READING HEADERS STATE: (TODO: typedef for the header as may include extra info)
		* In units of int32_t (i.e. 4 bytes) check the header value - if is a valid sized
		* instruction, enter reading data mode (up to size specified)
		*
		* If a header hasn't been fully read, call for another packet and complete
		*/
		if (state.state == REC_READING_HEADERS)
		{
			int32_t header = -1;
			uint8_t* header_ptr = (uint8_t*)&header;

			//Read in each byte and if havent finished, recieve again and finish
			//Assume the endianness of the system sending the header is the same (as probably is localhost)
			//TODO: Account for endianness in future
			for (size_t h_bytes_read = 0; h_bytes_read < sizeof(int32_t); ++h_bytes_read)
			{
				if (state.rec_offset >= state.incoming_size)
				{
					wimp_reciever_next_packet(&state, recsock, recbuffer);
				}
				header_ptr[h_bytes_read] = recbuffer[state.rec_offset];
				state.rec_offset++;
			}

			//Check values of the header
			assert(header != -1 && "Header value wasn't changed!");

			//If header is zero, are at end of data
			if (header == 0)
			{
				state.state = REC_IDLE;
			}
			else if (header != WIMP_RECIEVER_PING)
			{
				//If isn't a ping, create the instruction here and reading
				state.instruction = wimp_reciever_allocateinstr(header);
				state.state = REC_READING_DATA;

				//Add header
				memcpy(&state.instruction.instruction[0], &header, sizeof(header));
				state.instruction_bytes_read = sizeof(header);
			}
		}

		/*
		* READING DATA STATE: Read until the bytes read = the instructions read
		* Once the instruction is read, add to queue and return to header reading
		*
		* If didn't get all of instruction, get another packet
		*/
		if (state.state == REC_READING_DATA)
		{
			while (state.instruction_bytes_read != state.instruction.instruction_bytes)
			{
				//Copy up to end of recieved data
				size_t bytes_to_copy = state.instruction.instruction_bytes - state.instruction_bytes_read;

				if (state.rec_offset + bytes_to_copy > state.incoming_size)
				{
					bytes_to_copy =  state.incoming_size - state.rec_offset;
				}

				memcpy(&state.instruction.instruction[state.instruction_bytes_read], &recbuffer[state.rec_offset], bytes_to_copy);
				state.rec_offset += bytes_to_copy;
				state.instruction_bytes_read += bytes_to_copy;

				if (state.instruction_bytes_read != state.instruction.instruction_bytes)
				{
					wimp_reciever_next_packet(&state, recsock, recbuffer);
				}
			}

			//Check for the exit signal
			//Will be the "exit" instruction and this process will be the destination
			WimpInstrMeta meta = wimp_instr_get_from_buffer(state.instruction.instruction, state.instruction.instruction_bytes);
			if (strcmp(meta.instr, "exit") == 0 && strcmp(meta.dest_process, args->process_name) == 0)
			{
				disconnect = true;
			}

			//Lock queue and add instructions
			wimp_instr_queue_low_prio_lock(args->incoming_queue);
			wimp_instr_queue_add(args->incoming_queue, state.instruction.instruction, state.instruction.instruction_bytes);
			wimp_instr_queue_low_prio_unlock(args->incoming_queue);

			//Go back to reading headers and reset instr
			state.instruction.instruction = NULL;
			state.instruction.instruction_bytes = 0;
			state.instruction_bytes_read = 0;
			state.state = REC_READING_HEADERS;
		}
	}

	WIMP_ZERO_BUFFER(recbuffer);
    p_socket_address_free(rec_address);
    p_socket_free(recsock);
	wimp_free_reciever_args(args);
	return;
}

int32_t wimp_start_reciever_thread(const char* recfrom_name, const char* process_domain, int32_t process_port, RecieverArgs args)
{
	wimp_log("Starting Reciever for %s recieving from %s\n", args->process_name, recfrom_name);

	PUThread* process_thread = p_uthread_create((PUThreadFunc)&wimp_reciever_recieve, args, false, args->process_name);
	if (process_thread == NULL)
	{
		wimp_log_fail("Failed to create thread for %s reciever!\n", args->process_name);
		return WIMP_RECIEVER_FAIL;
	}
	return WIMP_RECIEVER_SUCCESS;
}

WimpInstr wimp_reciever_allocateinstr(pssize size)
{
	WimpInstr instr = { NULL, 0 };
	void* i = malloc(size);
	if (i == NULL)
	{
		return instr;
	}
	
	instr.instruction = i;
	instr.instruction_bytes = size;
	return instr;
}