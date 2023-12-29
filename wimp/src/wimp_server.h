#ifndef WIMP_SERVER_H
#define WIMP_SERVER_H

#include <stdbool.h>
#include <wimp_process_table.h>

#define WIMP_SERVER_SUCCESS 0
#define WIMP_SERVER_FAIL -1
#define WIMP_SERVER_ACCEPT_TIMEOUT 5000 //Waits 5000 ms before timing out on the blocking calls

typedef struct _WimpServer
{
	PSocketAddress* addr;
	PSocket* server;
	WimpProcessTable ptable;

	//Buffers for writing/reading
	WimpMsgBuffer recbuffer;
	WimpMsgBuffer sendbuffer;

} WimpServer;

/*
* Creates a Wimp Server
* 
* @param server The pointer to the server to create
* @param domain The domain for the server to run on
* @param port The port for the server to run on
* 
* @return Returns either WIMP_SERVER_SUCCESS or WIMP_SERVER_FAIL
*/
int32_t wimp_create_server(WimpServer* server, const char* domain, int32_t port);

/*
* Accepts a process to the server. Is blocking. Can only do one at a time.
* 
* @param server The server to accept a connection to
* @param process_name The name of the expected process for validation
* 
* @return Returns either WIMP_SERVER_SUCCESS or WIMP_SERVER_FAIL
*/
int32_t wimp_server_process_accept(WimpServer* server, const char* process_name);

/*
* Validates whether a given connection is still active by making a zero byte send call
* 
* @param server The server to accept a connection to
* @param process_name The name of the expected process for validation
* 
* @return Returns true if the connection exists still, false otherwise
*/
bool wimp_server_validate_process(WimpServer* server, const char* process_name);

/*
* Frees the Wimp Server
* 
* @param server The server to free
*/
void wimp_server_free(WimpServer server);

#endif