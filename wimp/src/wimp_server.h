/**
 * @file
 *
 * This header defines the interfaces to the wimp_server
 */

#ifndef WIMP_SERVER_H
#define WIMP_SERVER_H

#include <stdbool.h>
#include <wimp_core.h>
#include <wimp_process_table.h>
#include <wimp_instruction.h>
#include <wimp_log.h>

#define WIMP_SERVER_SUCCESS 0
#define WIMP_SERVER_FAIL -1
#define WIMP_SERVER_ACCEPT_TIMEOUT 5000 //Waits 5000 ms before timing out on the blocking calls

typedef int32_t WimpServerType;

typedef struct _WimpServer
{
	sds process_name;
	PSocketAddress* addr;
	PSocket* server;
	WimpProcessTable ptable;
	const char* parent;

	//Ingoing and outgoing msg queues
	WimpInstrQueue incomingmsg;
	WimpInstrQueue outgoingmsg;

	//Buffers for writing/reading
	WimpMsgBuffer sendbuffer;
	WimpMsgBuffer recbuffer;

	int32_t active;

} WimpServer;

/*
* Gets the local thread server
* 
* @return Returns the handle to the threads local server. Is null if uninitialized.
*/
WIMP_API WimpServer* wimp_get_local_server(void);

/*
* Initializes the local thread server. Must be closed afterwards. The local thread
* server provides an easy point of contact for sending instructions.
* 
* @param process_name The name of the process running on the server
* @param domain The domain for the server to run on
* @param port The port for the server to run on
* @param parent The parent process name (e.g. master process) - may be NULL if is not a child
* 
* @return Returns either WIMP_SERVER_SUCCESS or WIMP_SERVER_FAIL
*/
WIMP_API int32_t wimp_init_local_server(const char* process_name, const char* domain, int32_t port);

/*
* Closes the local thread server.
*/
WIMP_API void wimp_close_local_server(void);

/*
* Adds instructions to the local server
* 
* @param dest The name of the destination process
* @param instr The name of the instruction
* @param instr_size The size of the allocated instruction
*/
WIMP_API void wimp_add_local_server(const char* dest, const char* instr, const void* args, size_t arg_size_bytes);

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
WIMP_API int32_t wimp_create_server(WimpServer* server, const char* process_name, const char* domain, int32_t port);

/*
* Accepts a process to the server. Is blocking. Can only do one at a time.
* 
* @param server The server to accept a connection to
* @param pcount The number of processes to accept
* @param ... The names of the processes to accept
* 
* @return Returns either WIMP_SERVER_SUCCESS or WIMP_SERVER_FAIL
*/
WIMP_API int32_t wimp_server_process_accept(WimpServer* server, int pcount, ...);

/*
* Validates whether a given connection is still active by making a zero byte send call
* 
* @param server The server to accept a connection to
* @param process_name The name of the expected process for validation
* 
* @return Returns true if the connection exists still, false otherwise
*/
WIMP_API bool wimp_server_check_process_listening(WimpServer* server, const char* process_name);

/*
* Adds instructions to the server
* 
* @param server The server to add to
* @param dest The name of the destination process
* @param instr The name of the instruction
* @param instr_size The size of the allocated instruction
*/
WIMP_API void wimp_server_add(WimpServer* server, const char* dest, const char* instr, const void* args, size_t arg_size_bytes);

/*
* Awaits the server to recieve a specific instruction. Blocks and must not be
* called when queue mutexes are already locked!
* 
* @param server The server to await the response to
* @param instr The instruction to await
* @param timeout The timeout in milliseconds before returning back
* 
* @return Returns the node of the awaited instruction. Returns NULL if failed.
* The node should be freed after using.
*/
WIMP_API WimpInstrNode wimp_server_wait_response(WimpServer* server, const char* instr, int32_t timeout);

/*
* Routes server instructions if they don't belong to this server. If instruction
* is successfully routed (returns true) do not try to free the instr node as
* ownership is passed to the outgoing queue
* 
* @param server The server to route with
* @param dest_process The destination process of the instruction
* @param instrnode The node to route
* 
* @returns Returns true if the instruction was routed, otherwise false
*/
WIMP_API bool wimp_server_instr_routed(WimpServer* server, const char* dest_process, WimpInstrNode instrnode);

/*
* Sends the list of outgoing instructions to the desired processes.
* 
* @param server The server to send instructions from
*/
WIMP_API int32_t wimp_server_send_instructions(WimpServer* server);

/*
* Checks if the parent is alive (if the server has one). Can be used
* as a failsafe to make sure program exits if the parent is killed
* early and cannot clean up and send the exit signal. Should call in
* the main loop of a program that might be a separate executable.
* 
* @param server The server to check
* 
* @return Returns true if the parent is alive, or the process doesn't
* have a parent. Otherwise returns false.
*/
WIMP_API bool wimp_server_is_parent_alive(WimpServer* server);

/*
* Frees the Wimp Server
* 
* @param server The server to free
*/
WIMP_API void wimp_server_free(WimpServer* server);

#endif