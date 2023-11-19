#include <uni_socket.h>

static bool _socket_system_initialized = false;

int32_t init_uni_socket_system(void)
{
#ifdef _WIN32

	//Initialize Winsock.
    WSADATA wsaData;

    int32_t iResult;

    // Initialise Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != UNI_SOCKET_SUCCESS)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return iResult;
    }

    _socket_system_initialized = true;
    return iResult;

#else
    //TODO implement linux version
#endif

}

int32_t shutdown_uni_socket_system(void)
{
#ifdef _WIN32
    WSACleanup();
    _socket_system_initialized = false;
    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t create_uni_socket(const char* nodename, UniSocket* uni_socket, UniAddrInfo* addr_info, int32_t port, int32_t family, int32_t type, int32_t protocol, int32_t flags)
{
#ifdef _WIN32
    struct addrinfo *result = NULL, *ptr = NULL, hints;

    int32_t iResult;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_flags = flags;

    //Convert port number to the string it wants
    char port_id[MAX_PORT_CHARACTER_LENGTH];
    snprintf(port_id, MAX_PORT_CHARACTER_LENGTH, "%d", port);

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(nodename, port_id, &hints, &result);
    if (iResult != 0) 
    {
        printf("getaddrinfo failed: %d\n", iResult);
        return iResult;
    }

    // Create socket object
    *uni_socket = INVALID_UNI_SOCKET;

    // Create a SOCKET for the server to listen for client connections
    *uni_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (*uni_socket == INVALID_UNI_SOCKET) 
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return -1;
    }

    *addr_info = result;
    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t bind_uni_socket(UniSocket* uni_socket, UniAddrInfo addr_info)
{
#ifdef _WIN32
    // Setup the TCPlistening socket
    int32_t iResult = bind(*uni_socket, addr_info->ai_addr, (int32_t)addr_info->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(*uni_socket);
        *uni_socket = INVALID_SOCKET; //Invalidate the socket to avoid ambiguity
        return iResult;
    }
    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t listen_uni_socket(UniSocket uni_socket)
{
#ifdef _WIN32
    if (listen(uni_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        return SOCKET_ERROR;
    }
    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t connect_server_uni_socket(UniSocket* uni_socket, UniAddrInfo addr_info)
{
#ifdef _WIN32
    //Connect to the server.
	int32_t iResult = connect(*uni_socket, addr_info->ai_addr, (int32_t)addr_info->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(*uni_socket);
		*uni_socket = INVALID_UNI_SOCKET;
	}

    if (*uni_socket == INVALID_UNI_SOCKET)
	{
		printf("Unable to connect to server!\n");
		return 1;
	}

    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t accept_client_uni_socket(UniSocket server_socket, UniSocket* client_socket)
{
#ifdef _WIN32
    *client_socket = accept(server_socket, NULL, NULL);
    if (*client_socket == INVALID_UNI_SOCKET)
    {
        printf("accept failed: %d\n", WSAGetLastError());
        return 1;
    }
    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t shutdown_uni_socket_connection(UniSocket client_socket, int32_t connect_type)
{
#ifdef _WIN32
    int32_t iResult = shutdown(client_socket, connect_type);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        return iResult;
    }

    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t TEST_EXCHANGE_LOOP(UniSocket server_socket, UniSocket client_socket)
{
#ifdef _WIN32
    int8_t recvbuf[UNI_SOCKET_DEFAULT_SERVER_BUFLEN];
    int32_t iSendResult;
    int32_t recvbuflen = UNI_SOCKET_DEFAULT_SERVER_BUFLEN;
    int32_t iResult;

    do
    {
        iResult = recv(client_socket, recvbuf, recvbuflen, 0);
        if (iResult)
        {
            printf("Bytes recieved: %d\n", iResult);

            // Echo the buffer back to the sender
            iSendResult = send(client_socket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR)
            {
                printf("send failed: %d\n", WSAGetLastError());
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
        {
            printf("Connection closing\n");
        }
        else
        {
            printf("recv failed: %d\n", WSAGetLastError());
            return 1;
        }
    }
    while (iResult);

    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t close_uni_socket(UniSocket uni_socket)
{
#ifdef _WIN32
    closesocket(uni_socket);
    return UNI_SOCKET_SUCCESS;
#else

#endif
}

int32_t free_uni_addr_info(UniAddrInfo addr_info)
{
#ifdef _WIN32
    freeaddrinfo(addr_info);
    return UNI_SOCKET_SUCCESS;
#else

#endif
}