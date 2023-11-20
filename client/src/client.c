#include <uni_socket.h>
#include <stdio.h>
#include <stdint.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27015

int main(int argc, char** argv)
{
	//Initialize Winsock.
    if (init_uni_socket_system() != 0)
    {
        return 1;
    }

	//Create a socket.
    UniSocket ConnectSocket;
    UniAddrInfo result;
    if (create_uni_socket(argv[1], &ConnectSocket, &result, DEFAULT_PORT, AF_INET, SOCK_STREAM, IPPROTO_TCP, 0) != 0)
    {
        shutdown_uni_socket_system();
        return 1;
    }

	//Connect to the server.
	if (connect_server_uni_socket(&ConnectSocket, result) != UNI_SOCKET_SUCCESS)
	{
		shutdown_uni_socket_system();
		free_uni_addr_info(result);
		return 1;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message
	free_uni_addr_info(result);

	//Once connected to the server, loop until tell to disconnect
	bool disconnect = false;
	while (!disconnect)
	{
		char messageBuffer[DEFAULT_BUFLEN];
		printf("Input: ");
		fgets(messageBuffer, DEFAULT_BUFLEN, stdin);

		printf("Input recieved: %s\n", messageBuffer);

		//Once message recieved, send to the server
		int32_t iSendResult = send(ConnectSocket, messageBuffer, (int)strlen(messageBuffer), 0);
		if (iSendResult == SOCKET_ERROR)
		{
			printf("send failed: %d\n", WSAGetLastError());
		}

		//Wait for the response
		char recvbuf[DEFAULT_BUFLEN];
		int32_t iRecieveResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
		if (iRecieveResult > 0)
		{
			printf("Bytes received: %d\n", iRecieveResult);
			printf("Message recieved: %.*s\n", iRecieveResult, recvbuf);

			if (iRecieveResult == 9 && strncmp(recvbuf, "shutdown", 8) == 0)
			{
				disconnect = true;
			}
		}
	}

	// cleanup
	close_uni_socket(ConnectSocket);
	shutdown_uni_socket_system();

	return 0;
}