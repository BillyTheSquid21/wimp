#include <wimp_server.h>

int32_t wimp_create_server(WimpServer* server, const char* domain, int32_t port)
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

	server->addr = addr;
	server->ptable = ptable;
	server->server = s;

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

void wimp_server_free(WimpServer server)
{
	p_socket_address_free(server.addr);
	p_socket_close(server.server, NULL);
	wimp_process_table_free(server.ptable);
	WIMP_ZERO_BUFFER(server.recbuffer); WIMP_ZERO_BUFFER(server.sendbuffer);
}