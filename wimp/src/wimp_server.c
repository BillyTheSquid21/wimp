#include <wimp_server.h>
#include <utility/thread_local.h>

/*
* A thread can have a local server instance to make sending instructions
* easier.
*/
static thread_local WimpServer* _local_server = NULL;

WimpServer* wimp_get_local_server()
{
	return _local_server;
}

int32_t wimp_init_local_server(const char* process_name, const char* domain, int32_t port)
{
	if (_local_server != NULL)
	{
		printf("ERROR: Local server instance already exists!\n");
		return WIMP_SERVER_FAIL;
	}

	_local_server = malloc(sizeof(WimpServer));
	return wimp_create_server(_local_server, process_name, domain, port);
}

void wimp_close_local_server()
{
	if (_local_server == NULL)
	{
		printf("ERROR: Local server instance doesn't exist!\n");
		return WIMP_SERVER_FAIL;
	}
	wimp_server_free(*_local_server);
	_local_server = NULL;
}

void wimp_send_local_server(const char* dest, const char* instr, void* args, size_t arg_size_bytes)
{
	if (_local_server == NULL)
	{
		printf("ERROR: Local server instance doesn't exist!\n");
		return;
	}

	//Work out formatted size
	size_t destp_bytes = (strlen(dest) + 1) * sizeof(char);
	size_t sourcep_bytes = (strlen(_local_server->process_name) + 1) * sizeof(char);
	size_t instr_bytes = (strlen(instr) + 1) * sizeof(char);
	size_t arglen_bytes = sizeof(int32_t);
	size_t total_bytes = sourcep_bytes + destp_bytes + instr_bytes + arglen_bytes + arg_size_bytes;

	//Create a buffer and add the instructions
	uint8_t* instrbuff = malloc(total_bytes);
	if (instrbuff == NULL)
	{
		return;
	}

	size_t offset = 0;

	memcpy(&instrbuff[offset], dest, destp_bytes);
	offset += destp_bytes;

	memcpy(&instrbuff[offset], _local_server->process_name, sourcep_bytes);
	offset += sourcep_bytes;

	memcpy(&instrbuff[offset], instr, instr_bytes);
	offset += instr_bytes;

	memcpy(&instrbuff[offset], &arg_size_bytes, arglen_bytes);
	offset += arglen_bytes;

	if (arg_size_bytes > 0)
	{
		memcpy(&instrbuff[offset], args, arg_size_bytes);
	}

	wimp_instr_queue_add(&_local_server->outgoingmsg, instrbuff, total_bytes);
}

int32_t wimp_create_server(WimpServer* server, const char* process_name, const char* domain, int32_t port)
{
	WimpProcessTable ptable = wimp_create_process_table();

	WIMP_ZERO_BUFFER(server->recbuffer); WIMP_ZERO_BUFFER(server->sendbuffer);

	PSocketAddress* addr;
	PSocket* s;

	if ((addr = p_socket_address_new(domain, port)) == NULL)
	{
		return WIMP_SERVER_FAIL;
	}

	if ((s = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		p_socket_address_free(addr);
		return WIMP_SERVER_FAIL;
	}

	if (!p_socket_bind(s, addr, FALSE, NULL))
	{
		p_socket_address_free(addr);
		p_socket_free(s);

		return WIMP_SERVER_FAIL;
	}

	server->process_name = process_name;
	server->addr = addr;
	server->ptable = ptable;
	server->server = s;
	server->incomingmsg = wimp_create_instr_queue();
	server->outgoingmsg = wimp_create_instr_queue();

	return WIMP_SERVER_SUCCESS;
}

int32_t wimp_server_process_accept(WimpServer* server, const char* process_name)
{
	if (process_name == NULL)
	{
		return WIMP_SERVER_FAIL;
	}

	if (!p_socket_listen(server->server, NULL))
	{
		return WIMP_SERVER_FAIL;
	}

	printf("WAITING FOR CONNECTION...\n");

	//Ensures won't block for too long
	p_socket_set_timeout(server->server, WIMP_SERVER_ACCEPT_TIMEOUT);
	PSocket* con = p_socket_accept(server->server, NULL);

	if (con != NULL)
	{
		//Only blocking call, recieve handshake from reciever
		int handshake_size = p_socket_receive(con, server->recbuffer, WIMP_MESSAGE_BUFFER_BYTES, NULL);
			
		if (handshake_size <= 0)
		{
			printf("HANDSHAKE FAILED!\n");
			return WIMP_SERVER_FAIL;
		}

		//Check start of handshake
		WimpHandshakeHeader potential_handshake = *((WimpHandshakeHeader*)((void*)server->recbuffer));
		size_t offset = sizeof(WimpHandshakeHeader);
		if (potential_handshake.handshake_header != WIMP_RECIEVER_HANDSHAKE)
		{
			printf("HANDSHAKE FAILED!\n");
			return WIMP_SERVER_FAIL;
		}
		printf("test_process HANDSHAKE SUCCESS!\n");

		//Get process name
		char* proc_name = &server->recbuffer[offset];

		//Check if it is the expected process
		if (strcmp(proc_name, process_name) != 0)
		{
			printf("Incoming process is not the expected process!\n");
			return WIMP_SERVER_FAIL;
		}

		//Add connection to the process table
		WimpProcessData procdat = NULL;
		printf("Adding to test_process process table: %s\n", proc_name);
		if (wimp_process_table_get(&procdat, server->ptable, proc_name) == WIMP_PROCESS_TABLE_FAIL)
		{
			printf("Process not found! %s\n", proc_name);
			return WIMP_SERVER_FAIL;
		}
		printf("Process added!\n");

		procdat->process_connection = con;
		procdat->process_active = WIMP_PROCESS_ACTIVE;

		//Clear rec buffer
		WIMP_ZERO_BUFFER(server->recbuffer);

		//Send handshake back with no process name this time
		WimpHandshakeHeader* sendheader = ((WimpHandshakeHeader*)server->sendbuffer);
		sendheader->handshake_header = WIMP_RECIEVER_HANDSHAKE;
		sendheader->process_name_bytes = 0;

		p_socket_send(con, server->sendbuffer, sizeof(WimpHandshakeHeader), NULL);
		WIMP_ZERO_BUFFER(server->sendbuffer);
		return WIMP_SERVER_SUCCESS;
	}
	else
	{
		printf("Can't make con, tried and failed...\n");
		return WIMP_SERVER_FAIL;
	}
	return WIMP_SERVER_FAIL;
}

bool wimp_server_validate_process(WimpServer* server, const char* process_name)
{
	WimpProcessData procdat;
	if (wimp_process_table_get(&procdat, server->ptable, process_name) == WIMP_PROCESS_TABLE_FAIL)
	{
		return false;
	}

	if (!procdat->process_active)
	{
		return false;
	}

	int32_t ping = WIMP_RECIEVER_PING;
	if (p_socket_send(procdat->process_connection, &ping, sizeof(int32_t), NULL) != -1)
	{
		return true;
	}

	procdat->process_active = false;
	wimp_process_table_remove(&server->ptable, process_name);
	return false;
}

int32_t wimp_server_send_instructions(WimpServer* server)
{
	WimpInstrNode currentn = wimp_instr_queue_next(&server->outgoingmsg);
	while (currentn != NULL)
	{
		//Get process con - as starts with null term string can use start
		//of buffer as lookup
		WimpProcessData data = NULL;
		if (wimp_process_table_get(&data, server->ptable, currentn->instruction) == WIMP_PROCESS_TABLE_SUCCESS)
		{
			printf("Sending instr to: %s\n", currentn->instruction);
			memcpy(server->sendbuffer, currentn->instruction, currentn->instruction_bytes);
			p_socket_send(data->process_connection, server->sendbuffer, currentn->instruction_bytes, NULL);
			WIMP_ZERO_BUFFER(server->sendbuffer);
		}
		wimp_instr_node_free(currentn);
		currentn = wimp_instr_queue_next(&server->outgoingmsg);
	}

	return WIMP_SERVER_SUCCESS;
}

void wimp_server_free(WimpServer server)
{
	p_socket_address_free(server.addr);
	p_socket_close(server.server, NULL);
	wimp_process_table_free(server.ptable);
	wimp_instr_queue_free(server.incomingmsg);
	wimp_instr_queue_free(server.outgoingmsg);
	WIMP_ZERO_BUFFER(server.recbuffer); WIMP_ZERO_BUFFER(server.sendbuffer);
}