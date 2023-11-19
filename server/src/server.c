#include <stdio.h>
#include <stdint.h>
#include <uni_socket.h>

#define DEFAULT_PORT 27015

int main(void) 
{
    printf("Starting Server\n");

    //Initialize Winsock.
    if (init_uni_socket_system() != 0)
    {
        return 1;
    }

    //Create a socket.
    UniSocket ListenSocket;
    UniAddrInfo result;
    if (create_uni_socket(NULL, &ListenSocket, &result, DEFAULT_PORT, AF_INET, SOCK_STREAM, IPPROTO_TCP, AI_PASSIVE) != 0)
    {
        shutdown_uni_socket_system();
        return 1;
    }

    //Bind the socket.
    if (bind_uni_socket(&ListenSocket, result) != 0)
    {
        shutdown_uni_socket_system();
        return 1;
    }
    free_uni_addr_info(result);

    //Listen on the socket for a client.
    if (listen_uni_socket(ListenSocket) != 0)
    {
        shutdown_uni_socket_system();
        close_uni_socket(ListenSocket);
        return 1;
    }

    printf("Waiting for connection\n");
    //Accept a connection from a client.
    UniSocket ClientSocket = INVALID_UNI_SOCKET;
    if (accept_client_uni_socket(ListenSocket, &ClientSocket) != UNI_SOCKET_SUCCESS)
    {
        close_uni_socket(ListenSocket);
        shutdown_uni_socket_system();
        return 1;
    }
    printf("Client connected!\n");

    //Receive and send data.
    if (TEST_EXCHANGE_LOOP(ListenSocket, ClientSocket) != UNI_SOCKET_SUCCESS)
    {
        shutdown_uni_socket_system();
        close_uni_socket(ListenSocket);
    }

    //Disconnect.
    // shutdown the send half of the connection since no more data will be sent
    shutdown_uni_socket_connection(ClientSocket, SEND_UNI_SOCKET);

    // cleanup
    close_uni_socket(ClientSocket);
    close_uni_socket(ListenSocket);
    shutdown_uni_socket_system();

    return 0;
}