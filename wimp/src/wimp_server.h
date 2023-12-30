#ifndef WIMP_SERVER_H
#define WIMP_SERVER_H

#include <stdbool.h>
#include <wimp_process_table.h>
#include <wimp_instruction.h>

#define WIMP_SERVER_SUCCESS 0
#define WIMP_SERVER_FAIL -1
#define WIMP_SERVER_ACCEPT_TIMEOUT 5000 //Waits 5000 ms before timing out on the blocking calls

typedef struct _WimpServer
{
	const char* process_name;
	PSocketAddress* addr;
	PSocket* server;
	WimpProcessTable ptable;

	//Ingoing and outgoing msg queues
	WimpInstrQueue incomingmsg;
	WimpInstrQueue outgoingmsg;

	//Buffers for writing/reading
	WimpMsgBuffer sendbuffer;
	WimpMsgBuffer recbuffer;

} WimpServer;

/*
* Gets the local thread server
* 
* @return Returns the handle to the threads local server. Is null if uninitialized.
*/
WimpServer* wimp_get_local_server();

/*
* Initializes the local thread server. Must be closed afterwards. The local thread
* server provides an easy point of contact for sending instructions.
* 
* @param process_name The name of the process running on the server
* @param domain The domain for the server to run on
* @param port The port for the server to run on
* 
* @return Returns either WIMP_SERVER_SUCCESS or WIMP_SERVER_FAIL
*/
int32_t wimp_init_local_server(const char* process_name, const char* domain, int32_t port);

/*
* Closes the local thread server.
*/
void wimp_close_local_server();

/*
* Sends instructions from the local server to another server via a reciever.
* Should be allocated in a function specific to the API being called.
* 
* @param dest The name of the destination process
* @param instr The name of the instruction
* @param instr_size The size of the allocated instruction
*/
void wimp_send_local_server(const char* dest, const char* instr, void* args, size_t arg_size_bytes);

/*
* Creates a Wimp Server instance. This can alternatively be used to create a non
* thread local server, however the server handle will have to be passed around
* to send instructions. Should also be freed after.
* 
* @param server The pointer to the server to create
* @param process_name The name of the process running on the server
* @param domain The domain for the server to run on
* @param port The port for the server to run on
* 
* @return Returns either WIMP_SERVER_SUCCESS or WIMP_SERVER_FAIL
*/
int32_t wimp_create_server(WimpServer* server, const char* process_name, const char* domain, int32_t port);

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
* Sends the list of outgoing instructions to the desired processes.
* 
* @param server The server to send instructions from
*/
int32_t wimp_server_send_instructions(WimpServer* server);

/*
* Frees the Wimp Server
* 
* @param server The server to free
*/
void wimp_server_free(WimpServer server);

#endif