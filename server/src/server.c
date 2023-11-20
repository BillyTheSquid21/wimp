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
    //free_uni_addr_info(result);

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

    //Wait to recieve data, then do stuff with it
    bool disconnect = false;
    while (!disconnect)
    {
        int8_t recvbuf[UNI_SOCKET_DEFAULT_SERVER_BUFLEN];
        int32_t recvbuflen = UNI_SOCKET_DEFAULT_SERVER_BUFLEN;
        int32_t iRecieveResult;

        int8_t sendvbuf[UNI_SOCKET_DEFAULT_SERVER_BUFLEN];
        int32_t sendbuflen = UNI_SOCKET_DEFAULT_SERVER_BUFLEN;
        int32_t iSendResult;

        //Zero both buffers
        ZeroMemory(&recvbuf, UNI_SOCKET_DEFAULT_SERVER_BUFLEN);
        ZeroMemory(&sendvbuf, UNI_SOCKET_DEFAULT_SERVER_BUFLEN);

        iRecieveResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iRecieveResult > 0)
        {
            printf("Bytes recieved: %d\n", iRecieveResult);
            printf("Message: %s\n", recvbuf);

            //Check content of buffer against commands
            if (iRecieveResult == 11 && strncmp(recvbuf, UNI_SOCKET_DISCONNECT_FROM_SERVER, 10) == 0)
            {
                printf("Disconnect\n");
                //Tell to shut down
                iSendResult = send(ClientSocket, "shutdown", 9, 0);
                if (iSendResult == SOCKET_ERROR)
                {
                    printf("send failed: %d\n", WSAGetLastError());
                }

                //Disconnect from Client socket and set disconnect to true
                close_uni_socket(ClientSocket);
                disconnect = true;
            }
            else if (iRecieveResult >= 5 && strncmp(recvbuf, UNI_SOCKET_ECHO_MESSAGE, 4) == 0)
            {
                //Check there are characters from pos 5 to n (size at least 6)
                if (iRecieveResult >= 6)
                {
                    printf("Echo\n");
                    //Send the buffer from 5 to n, so size = iRecieveResult - 5
                    iSendResult = send(ClientSocket, &recvbuf[4], iRecieveResult - 5, 0);
                    if (iSendResult == SOCKET_ERROR)
                    {
                        printf("send failed: %d\n", WSAGetLastError());
                    }
                    printf("sent: %s\n", &recvbuf[4]);
                }
            }
            else if (iRecieveResult == 7 && strncmp(recvbuf, UNI_SOCKET_GET_DOMAIN, 6) == 0)
            {
                printf("Domain\n");
                //Send a new result containing the domain name string
                iSendResult = send(ClientSocket, "localhost", 10, 0);
                if (iSendResult == SOCKET_ERROR)
                {
                    printf("send failed: %d\n", WSAGetLastError());
                }
                printf("sent: %s\n", "localhost");
            }
            else
            {
                //Respond with unknown message error
                iSendResult = send(ClientSocket, "unknown message", 16, 0);
                if (iSendResult == SOCKET_ERROR)
                {
                    printf("send failed: %d\n", WSAGetLastError());
                }
            }
        }
    }

    free_uni_addr_info(result);

    // cleanup
    close_uni_socket(ListenSocket);
    shutdown_uni_socket_system();

    return 0;
}