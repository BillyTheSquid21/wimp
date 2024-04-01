#include <wimp_server.h>
#include <utility/thread_local.h>
#include <wimp_log.h>
#include <stdlib.h>

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
		wimp_log_fail("Local server instance already exists!\n");
		return WIMP_SERVER_FAIL;
	}

	_local_server = malloc(sizeof(WimpServer));
	return wimp_create_server(_local_server, process_name, domain, port);
}

void wimp_close_local_server()
{
	if (_local_server == NULL)
	{
		wimp_log_fail("Local server instance doesn't exist!\n");
		return;
	}
	wimp_server_free(*_local_server);
	_local_server = NULL;
}

void wimp_add_local_server(const char* dest, const char* instr, const void* args, size_t arg_size_bytes)
{
	if (_local_server == NULL)
	{
		wimp_log_fail("Local server instance doesn't exist!\n");
		return;
	}

	wimp_server_add(_local_server, dest, instr, args, arg_size_bytes);
}

int32_t wimp_create_server(WimpServer* server, const char* process_name, const char* domain, int32_t port)
{
	WimpProcessTable ptable = wimp_create_process_table();
	WIMP_ZERO_BUFFER(server->recbuffer); WIMP_ZERO_BUFFER(server->sendbuffer);

	PSocketAddress* addr;
	PSocket* s;
	PError* err;

	if ((addr = p_socket_address_new(domain, port)) == NULL)
	{
		return WIMP_SERVER_FAIL;
	}

	if ((s = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, &err)) == NULL)
	{
		wimp_log_fail("Failed to create server socket! (%d): %s\n", p_error_get_code(err), p_error_get_message(err));
		p_error_free(err);
		p_socket_address_free(addr);
		return WIMP_SERVER_FAIL;
	}

	if (!p_socket_bind(s, addr, TRUE, &err))
	{
		wimp_log_fail("Failed to bind server socket! (%d): %s\n", p_error_get_code(err), p_error_get_message(err));
		p_error_free(err);
		p_socket_address_free(addr);
		p_socket_free(s);
		return WIMP_SERVER_FAIL;
	}
	
	server->process_name = process_name;
	server->addr = addr;
	server->ptable = ptable;
	server->server = s;
	server->parent = NULL;
	server->incomingmsg = wimp_create_instr_queue();
	server->outgoingmsg = wimp_create_instr_queue();
	wimp_log_success("Server created! %s %s:%d\n", process_name, domain, port);
	return WIMP_SERVER_SUCCESS;
}

int32_t wimp_server_process_accept(WimpServer* server, int pcount, ...)
{
	//Get an array of the process names
	char** pnames;
	pnames = malloc(pcount * sizeof(char*));
	if (pnames == NULL)
	{
		return WIMP_SERVER_FAIL;
	}

	//Copy the vargs to the array to check with
	va_list argp;
	va_start(argp, pcount);
	for (int i = 0; i < pcount; ++i)
	{
		char* p = va_arg(argp, char*);
		pnames[i] = p;
	}
	va_end(argp);

	//Set the server to listen for incoming connections - should succeed
	if (!p_socket_listen(server->server, NULL))
	{
		return WIMP_SERVER_FAIL;
	}
	wimp_log("Server %s waiting to accept %d connections\n", server->process_name, pcount);

	//Ensures won't block for too long
	p_socket_set_timeout(server->server, WIMP_SERVER_ACCEPT_TIMEOUT);
	
	//Wait for pcount many connections to be made, perform checks/handshake
	int accepted_count = 0;
	for (int i = 0; i < pcount; ++i)
	{
		PError* err = NULL;
		PSocket* con = p_socket_accept(server->server, &err);

		if (con != NULL)
		{
			//Only blocking call, recieve handshake from reciever
			pssize handshake_size = p_socket_receive(con, server->recbuffer, WIMP_MESSAGE_BUFFER_BYTES, &err);
			
			if (handshake_size <= 0)
			{
				continue;
			}

			//Check start of handshake
			WimpHandshakeHeader potential_handshake = *((WimpHandshakeHeader*)((void*)server->recbuffer));
			size_t offset = sizeof(WimpHandshakeHeader);
			if (potential_handshake.handshake_header != WIMP_RECIEVER_HANDSHAKE)
			{
				continue;
			}

			//Get process name
			char* proc_name = &server->recbuffer[offset];

			//Check if it is an expected process
			bool isvalid = false;
			for (int j = 0; j < pcount; ++j)
			{
				if (strcmp(proc_name, pnames[j]) == 0)
				{
					wimp_log("Valid process found: %s\n", proc_name);
					isvalid = true;
					break;
				}
			}

			if (!isvalid)
			{
				wimp_log_important("An incoming connection wasn't a valid one! This may be malicious\n");
				i--; //Try again, hoping the next connection won't be bad
				continue;
			}

			//Add connection to the process table
			WimpProcessData procdat = NULL;
			wimp_log("Adding to %s process table: %s\n", server->process_name, proc_name);
			if (wimp_process_table_get(&procdat, server->ptable, proc_name) == WIMP_PROCESS_TABLE_FAIL)
			{
				wimp_log_fail("Process not found! %s\n", proc_name);
				continue;
			}
			wimp_log("Process added!\n");

			procdat->process_connection = con;
			procdat->process_active = WIMP_PROCESS_ACTIVE;

			if (procdat->process_relation == WIMP_Process_Parent)
			{
				if (server->parent == NULL)
				{
					server->parent = sdsnew(proc_name);
				}
				else
				{
					wimp_log_fail("Server already has a parent %s!\n", server->parent);
				}
			}

			//Clear rec buffer
			WIMP_ZERO_BUFFER(server->recbuffer);

			//Send handshake back with no process name this time
			WimpHandshakeHeader* sendheader = ((WimpHandshakeHeader*)server->sendbuffer);
			sendheader->handshake_header = WIMP_RECIEVER_HANDSHAKE;
			sendheader->process_name_bytes = 0;

			p_socket_send(con, server->sendbuffer, sizeof(WimpHandshakeHeader), NULL);
			WIMP_ZERO_BUFFER(server->sendbuffer);

			accepted_count++;
		}
		else
		{
			wimp_log_fail("Can't make connection (%d) %s\n", p_error_get_code(err), p_error_get_message(err));
			p_error_free(err);
		}
	}
	free(pnames);

	//Time out for 100ms for now to prevent user sending instructions until the handshake is cleared
	p_uthread_sleep(100);

	if (accepted_count != pcount)
	{
		wimp_log_fail("%s couldn't find every process!\n", server->process_name);
		return WIMP_SERVER_FAIL;
	}
	wimp_log_success("%s found every process!\n", server->process_name);
	return WIMP_SERVER_SUCCESS;
}

bool wimp_server_check_process_listening(WimpServer* server, const char* process_name)
{
	WimpProcessData procdat;
	if (wimp_process_table_get(&procdat, server->ptable, process_name) == WIMP_PROCESS_TABLE_FAIL)
	{
		wimp_log_fail("Process isn't in process table!: %s\n", process_name);
		return false;
	}

	if (!procdat->process_active)
	{
		wimp_log_fail("Process isn't active!: %s\n", process_name);
		return false;
	}

	PError* err = NULL;
	int32_t ping = WIMP_RECIEVER_PING;
	if (p_socket_send(procdat->process_connection, (const pchar*)&ping, sizeof(int32_t), NULL) != -1)
	{
		return true;
	}

	procdat->process_active = false;
	wimp_process_table_remove(&server->ptable, process_name);
	return false;
}

void wimp_server_add(WimpServer* server, const char* dest, const char* instr, const void* args, size_t arg_size_bytes)
{
	//Work out formatted size
	size_t header_bytes = sizeof(int32_t);
	size_t destp_bytes = (strlen(dest) + 1) * sizeof(char);
	size_t sourcep_bytes = (strlen(server->process_name) + 1) * sizeof(char);
	size_t instr_bytes = (strlen(instr) + 1) * sizeof(char);
	size_t arglen_bytes = sizeof(int32_t);
	size_t total_bytes = header_bytes + sourcep_bytes + destp_bytes + instr_bytes + arglen_bytes + arg_size_bytes;

	//Create a buffer and add the instructions
	uint8_t* instrbuff = malloc(total_bytes);
	if (instrbuff == NULL)
	{
		return;
	}

	size_t offset = 0;

	memcpy(&instrbuff[offset], &total_bytes, header_bytes);
	offset += header_bytes;

	memcpy(&instrbuff[offset], dest, destp_bytes);
	offset += destp_bytes;

	memcpy(&instrbuff[offset], server->process_name, sourcep_bytes);
	offset += sourcep_bytes;

	memcpy(&instrbuff[offset], instr, instr_bytes);
	offset += instr_bytes;

	memcpy(&instrbuff[offset], &arg_size_bytes, arglen_bytes);
	offset += arglen_bytes;

	if (arg_size_bytes > 0)
	{
		memcpy(&instrbuff[offset], args, arg_size_bytes);
	}

	wimp_instr_queue_add(&server->outgoingmsg, instrbuff, total_bytes);
}

bool wimp_server_instr_routed(WimpServer* server, const char* dest_process, WimpInstrNode instrnode)
{
	if (strcmp(dest_process, server->process_name) != 0)
	{
		//Add to the outgoing and continue to prevent freeing
		wimp_instr_queue_add_existing(&server->outgoingmsg, instrnode);
		return true;
	}
	return false;
}

int32_t wimp_server_send_instructions(WimpServer* server)
{
	wimp_instr_queue_high_prio_lock(&server->outgoingmsg);
	WimpInstrNode currentn = wimp_instr_queue_pop(&server->outgoingmsg);
	while (currentn != NULL)
	{
		//Get process con
		//of buffer as lookup
		WimpProcessData data = NULL;

		//get destination from after instr size offset
		char* destination = &currentn->instr.instruction[WIMP_INSTRUCTION_DEST_OFFSET];

		//If the destination is this server, add to incoming instead (loopback)
		if (strcmp(destination, server->process_name) == 0)
		{
			wimp_instr_queue_add(&server->incomingmsg, currentn->instr.instruction, currentn->instr.instruction_bytes);
		}
		else if 
			(
			//First look for a destination in the server ptable
			//Otherwise send to the default destination (the parent) which may route it
			//Short circuit is guaranteed by C standard
			wimp_process_table_get(&data, server->ptable, destination) == WIMP_PROCESS_TABLE_SUCCESS
			||
			wimp_process_table_get(&data, server->ptable, server->parent) == WIMP_PROCESS_TABLE_SUCCESS
			)
		{
			//Check the process is still active, otherwise scrap the instruction
			if (data->process_active)
			{
				//If a valid place to send to is found send the instruction
				memcpy(server->sendbuffer, currentn->instr.instruction, currentn->instr.instruction_bytes);
			
				WimpInstrMeta meta = wimp_instr_get_from_buffer(server->sendbuffer, WIMP_MESSAGE_BUFFER_BYTES);
				pssize sendres = p_socket_send(data->process_connection, server->sendbuffer, currentn->instr.instruction_bytes, NULL);

				//DEBUG_WIMP_PRINT_INSTRUCTION_META(meta);
				WIMP_ZERO_BUFFER(server->sendbuffer);
			}
		}
		wimp_instr_node_free(currentn);
		currentn = wimp_instr_queue_pop(&server->outgoingmsg);
	}
	wimp_instr_queue_high_prio_unlock(&server->outgoingmsg);
	return WIMP_SERVER_SUCCESS;
}

bool wimp_server_is_parent_alive(WimpServer* server)
{
	if (server->parent == NULL)
	{
		return true;
	}
	return wimp_server_check_process_listening(server, server->parent);
}

void wimp_server_free(WimpServer server)
{
	//Before freeing, send exit signal to any child process
	HashStringEntry* entry;
	int i;
	HASH_STRING_ITER(server.ptable._hash_table, entry, i)
	{
		WimpProcessData data = (WimpProcessData)entry->value;
		if (data->process_relation == WIMP_Process_Child)
		{
			wimp_server_add(&server, entry->key, WIMP_INSTRUCTION_EXIT, NULL, 0);
		}
	}
	wimp_server_send_instructions(&server);

	p_socket_address_free(server.addr);
	p_socket_close(server.server, NULL);
	wimp_process_table_free(server.ptable);
	wimp_instr_queue_free(server.incomingmsg);
	wimp_instr_queue_free(server.outgoingmsg);
	if (server.parent)
	{
		sdsfree(server.parent);
	}
	WIMP_ZERO_BUFFER(server.recbuffer); WIMP_ZERO_BUFFER(server.sendbuffer);
}