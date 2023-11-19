#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_PORT_CHARACTER_LENGTH 6
#define UNI_SOCKET_SUCCESS 0
#define UNI_SOCKET_DEFAULT_SERVER_BUFLEN 512

#ifdef _WIN32
#define _WIN32_WINNT 0x501
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET UniSocket;
typedef struct addrinfo* UniAddrInfo; //TODO - make universal interface

#define INVALID_UNI_SOCKET INVALID_SOCKET
#define SEND_UNI_SOCKET SD_SEND
#define RECIEVE_UNI_SOCKET SD_RECEIVE

#else
//TODO - eventually figure out what to do for linux
#endif

//Create a simple cross platform socket implementation
//As my sockets will be pretty simple should be ok for my uses
//Return value of zero represents a success of the function

//Initialize socket implementation
int32_t init_uni_socket_system(void);

//Shutdown the socket implementation
int32_t shutdown_uni_socket_system(void);

//Create a socket - TODO make additional info system less horrible
int32_t create_uni_socket(const char* nodename, UniSocket* uni_socket, UniAddrInfo* addr_info, int32_t port, int32_t family, int32_t type, int32_t protocol, int32_t flags);

//Bind the socket
int32_t bind_uni_socket(UniSocket* uni_socket, UniAddrInfo addr_info);

//Set state to listening
int32_t listen_uni_socket(UniSocket uni_socket);

//Connect to server socket
int32_t connect_server_uni_socket(UniSocket* uni_socket, UniAddrInfo addr_info);

//Accept connection from a client socket
int32_t accept_client_uni_socket(UniSocket server_socket, UniSocket* client_socket);

//Shut send or recieve connection
int32_t shutdown_uni_socket_connection(UniSocket client_socket, int32_t connect_type);

//Exchange data until a signal to stop is given - TODO dont do this
int32_t TEST_EXCHANGE_LOOP(UniSocket server_socket, UniSocket client_socket);

//Close uni socket
int32_t close_uni_socket(UniSocket uni_socket);

//Free the memory used by the addr_info TODO make not use the heap for storing, free in up instead
int32_t free_uni_addr_info(UniAddrInfo addr_info);