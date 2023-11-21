#include <uni_socket.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_MESSAGE_LENGTH 1024
#define SERVER_PORT 5432

int main(int argc, char** argv)
{
	p_libsys_init();
	char buffer[MAX_MESSAGE_LENGTH + 1]; // Supports messages up to max message length characters long (plus null character)

	PSocketAddress* addr;
	PSocket* sock;

	// Construct address for server.  Since the server is assumed to be on the same machine for the sake of this program, the address is loopback, but typically this would be an external address.
	if ((addr = p_socket_address_new("127.0.0.1", SERVER_PORT)) == NULL)
		return 1;

	// Create socket
	if ((sock = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		// Can't construct socket, cleanup and exit.
		p_socket_address_free(addr);

		return 2; 
	}


	// Connect to server.
	if (!p_socket_connect(sock, addr, NULL))
	{
		// Couldn't connect, cleanup.
		p_socket_address_free(addr);
		p_socket_free(sock);

		return 4;
	}

	// Receive our message and print.
	pssize sizeOfRecvData = p_socket_receive(sock, buffer, sizeof(buffer) - 1, NULL);
	buffer[sizeOfRecvData] = '\0'; // Set null character 1 after message

	printf("We received %s\n", buffer);

	// Cleanup
	p_socket_address_free(addr);
	p_socket_close(sock, NULL);
	p_libsys_shutdown();
	return 0;
}