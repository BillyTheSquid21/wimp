#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp_reciever.h>
#include <wimp_process.h>

#define MAX_MESSAGE_LENGTH 1024
#define SERVER_PORT 5432
#define END_PROCESS_PORT 5433

int client_main_entry(int argc, char** argv)
{
	//Process command line args - default values
	int32_t port = END_PROCESS_PORT;
	int32_t master_port = SERVER_PORT;

	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
		{
			port = strtol(argv[i+1], NULL, 10);
		}

		if (strcmp(argv[i], "--master-port") == 0 && i + 1 < argc)
		{
			master_port = strtol(argv[i+1], NULL, 10);
		}
	}

	char buffer[MAX_MESSAGE_LENGTH + 1]; // Supports messages up to max length (plus null character that terminates the string, since we're sending text)

	PSocketAddress* addr;
	PSocket* sock;

	// Binding socket to local host (we are a server, the appropriate port.  Typically this will always be a localhost port, because we are going to listen to this)
	if ((addr = p_socket_address_new("127.0.0.1", port)) == NULL)
		return 1;

	// Create socket
	if ((sock = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		// Failed to create socket -- cleanup
		p_socket_address_free(addr);

		return 2;
	}

	// Bind to local host (server) socket
	if (!p_socket_bind(sock, addr, FALSE, NULL))
	{
		// Couldn't bind socket, cleanup
		p_socket_address_free(addr);
		p_socket_free(sock);

		return 3;
	}

	// Listen for incoming connections on localhost SERVER_PORT
	if (!p_socket_listen(sock, NULL))
	{
		// Couldn't start listening, cleanup
		p_socket_address_free(addr);
		p_socket_close(sock, NULL);

		return 4;
	}

	// Forever, try to accept incoming connections.
	while (1)
	{
		printf("WAITING FOR CONNECTION...\n");
		// Blocks until connection accept happens by default -- this can be changed
		PSocket* con = p_socket_accept(sock, NULL);
		printf("Recieved from: %d\n", p_socket_get_fd(con));
		if (con != NULL)
		{
			printf("Sending message...\n");
			uint32_t* potential_handshake = ((uint32_t*)((void*)buffer));
			*potential_handshake = WIMP_RECIEVER_HANDSHAKE;
			p_socket_send(con, buffer, WIMP_RECIEVER_HANDSHAKE_LENGTH, NULL);

			//Send handshake again to trigger the write
			p_socket_send(con, buffer, WIMP_RECIEVER_HANDSHAKE_LENGTH, NULL);
			break;
		}
		else
			printf("Can't make con, tried and failed...\n");
	}

	// Cleanup
	printf("Server thread closed: %d\n", sock);
	p_socket_address_free(addr);
	p_socket_close(sock, NULL);
	return 0;
}

int client_main_lib_entry(MainEntry* entry)
{
	return client_main_entry(entry->argc, entry->argv);
}

int main(void) 
{
	p_libsys_init();

	// Start child process and launches a reciever that recieves from the end process
	int32_t writebuffer = 0;

	//Need a port for the master to send to the end reciever
	//And a port for the end process to send to the master reciever
	int32_t master_port = wimp_assign_unused_local_port();
	int32_t end_process_port = wimp_assign_unused_local_port();
	wimp_start_library_process("client", &client_main_lib_entry, "127.0.0.1", master_port, end_process_port, &writebuffer);
	
	// Then needs to create a sending thread for the child process

	// Forever, read if the data is written to the buffer
	while (writebuffer == 0)
	{
		printf("NOT WRITTEN\n");
		p_uthread_sleep(200);
	}
	printf("WRITTEN!");

	// Cleanup
	p_libsys_shutdown();
	return 0;
}