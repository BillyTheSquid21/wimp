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

	//Send and receive data.

	int recvbuflen = DEFAULT_BUFLEN;

	const char* sendbuf = "this is a test\n";
	const char* sendbuf2 = "this is another test\n";

	char recvbuf[DEFAULT_BUFLEN];

	// Send an initial buffer
	int32_t iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	iResult = send(ConnectSocket, sendbuf2, (int)strlen(sendbuf2), 0);

	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	shutdown_uni_socket_connection(ConnectSocket, SEND_UNI_SOCKET);

	// Receive data until the server closes the connection
	do
	{
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);
			printf("Message recieved: %.*s\n", iResult, recvbuf);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);

	// cleanup
	close_uni_socket(ConnectSocket);
	shutdown_uni_socket_system();

	return 0;
}