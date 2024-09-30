///
/// @file
///
/// This header defines the interfaces to the wimp_server
///

#ifndef WIMP_SERVER_H
#define WIMP_SERVER_H

#include <stdbool.h>
#include <wimp_core.h>
#include <wimp_process_table.h>
#include <wimp_instruction.h>
#include <wimp_log.h>

/// @brief The result of wimp server operations
enum WimpServerResult
{
    WIMP_SERVER_SUCCESS            =  0, ///< Result if server operation is successful
    WIMP_SERVER_FAIL                = -1,///< Result if server operation fails for an unspecified reason
    WIMP_SERVER_ADDRESS_FAIL       = -2, ///< Result if server fails to create a new address
    WIMP_SERVER_SOCKET_FAIL        = -3, ///< Result if server fails to create a new socket
    WIMP_SERVER_BIND_FAIL          = -4, ///< Result if server fails to bind it's socket
    WIMP_SERVER_LISTEN_FAIL        = -5, ///< Result if server socket fails to listen
    WIMP_SERVER_TOO_FEW_PROCESSES  = -6, ///< Result if fewer processes than expected attempt to accept
    WIMP_SERVER_UNEXPECTED_PROCESS = -7, ///< Result if an unexpected process attempts to accept
};

#define WIMP_SERVER_ACCEPT_TIMEOUT 5000 //Waits 5000 ms before timing out on the blocking calls

typedef int32_t WimpServerType;

///
/// @brief The struct containing the WIMP server information
///
/// Is created with wimp_create_server(...) and destroyed with wimp_server_free(...)
/// It is recommended to use a local server for most use cases as each thread should
/// really only have one server.
///
typedef struct _WimpServer
{
    sds process_name;       ///< Name of the server
    PSocketAddress* addr;   ///< Server address structure
    PSocket* server;        ///< Server socket pointer
    WimpProcessTable ptable;///< Process table tracking connected processes
    sds parent;             ///< Name of the parent process - is null when no parent exists

    //Ingoing and outgoing msg queues
    WimpInstrQueue incomingmsg; ///< Incoming message queue
    WimpInstrQueue outgoingmsg; ///< Outgoing message queue

    //Buffers for writing/reading
    WimpMsgBuffer sendbuffer; ///< Sending buffer
    WimpMsgBuffer recbuffer;  ///< Recieving buffer

    int32_t active; ///< The active status of the server

} WimpServer;

///
/// @brief Gets the local thread server
/// 
/// @return Returns the handle to the threads local server. Is null if uninitialized.
///
WIMP_API WimpServer* wimp_get_local_server(void);

/// 
/// @brief Initializes the local server
///
/// Server must be closed afterwards with wimp_close_local_server() .
/// The local thread server provides an easy point of contact for sending instructions.
/// 
/// @param process_name The name of the process running on the server
/// @param domain The domain for the server to run on
/// @param port The port for the server to run on
/// @param parent The parent process name (e.g. master process) - may be NULL if is not a child
/// 
/// @return Returns a WimpServerResult enum
///
WIMP_API int32_t wimp_init_local_server(const char* process_name, const char* domain, int32_t port);

///
/// @brief Closes the local server
///
WIMP_API void wimp_close_local_server(void);

///
/// @brief Adds instructions to the local server outgoing queue
/// 
/// @param dest The name of the destination process
/// @param instr The instruction
/// @param args Any additional arguments to be copied
/// @param arg_size_bytes The size of any arguments to copy
///
WIMP_API void wimp_add_local_server(const char* dest, uint64_t instr, const void* args, size_t arg_size_bytes);

///
/// @brief Starts a reciever thread for the local server
/// 
/// @param process_name The name of the process the local server runs on
/// @param process_domain The domain the local server runs on
/// @param process_port The port the local server runs on
/// @param recfrom_name The name of the process to recieve from
/// @param recfrom_domain The domain to recieve from
/// @param recfrom_port The port to recieve from
/// 
/// @return Returns a WimpRecieverResult enum
/// 
WIMP_API int32_t wimp_start_local_server_reciever_thread(const char* process_name, const char* process_domain, int32_t process_port, const char* recfrom_name, const char* recfrom_domain, int32_t recfrom_port);

///
/// @brief Locks the local server incoming queue
/// 
WIMP_API void wimp_incoming_queue_local_server_lock();

///
/// @brief Unlocks the local server incoming queue
/// 
WIMP_API void wimp_incoming_queue_local_server_unlock();

///
/// @brief Pops the front instruction from the local server incoming queue
/// 
/// @return Returns the instruction node
/// 
WIMP_API WimpInstrNode wimp_incoming_queue_local_server_pop();

///
/// @brief Adds a new process to the local server
/// 
/// @param process_name The name of the process to add
/// @param process_domain The domain of the process to add
/// @param process_port The port of the process to add
/// @param relation The relation of the added process to this one
/// 
/// @return Returns a WimpProcessResult enum
/// 
WIMP_API int32_t wimp_add_local_server_process(const char* process_name, const char* process_domain, int32_t process_port, int32_t relation);

///
/// @brief Creates an instance of a WIMP server
///
/// This can be alternatively used to create a non thread local server, 
/// however the server handle will have to be passed around to send instructions. 
/// Should be freed after use with wimp_server_free().
/// 
/// @param server The pointer to the server to create
/// @param process_name The name of the process running on the server
/// @param domain The domain for the server to run on
/// @param port The port for the server to run on
/// 
/// @return Returns a WimpServerResult enum
///
WIMP_API int32_t wimp_create_server(WimpServer* server, const char* process_name, const char* domain, int32_t port);

///
/// @brief Accepts valid processes connecting to the server
///
/// Blocks until completion. Only allows the connections specified with ...
/// 
/// @param server The server to accept a connection to
/// @param pcount The number of processes to accept
/// @param ... The names of the processes to accept
/// 
/// @return Returns a WimpServerResult enum
///
WIMP_API int32_t wimp_server_process_accept(WimpServer* server, int pcount, ...);

///
/// @brief Checks if a process is still connected
///
/// Validates whether a given connection is still active by making a zero byte send call
/// 
/// @param server The server to accept a connection to
/// @param process_name The name of the expected process for validation
/// 
/// @return Returns true if the connection exists still, false otherwise
///
WIMP_API bool wimp_server_check_process_listening(WimpServer* server, const char* process_name);

///
/// @brief Adds instructions to the server outgoing queue
/// 
/// @param server The server to add to
/// @param dest The name of the destination process
/// @param instr The instruction
/// @param args Any additional arguments to be copied
/// @param arg_size_bytes The size of any arguments to copy
///
WIMP_API void wimp_server_add(WimpServer* server, const char* dest, uint64_t instr, const void* args, size_t arg_size_bytes);

///
/// @brief Waits until the specified instruction is recieved
///
/// Awaits the server to recieve a specific instruction. Blocks and must not be
/// called when queue mutexes are already locked!
/// 
/// @param server The server to await the response to
/// @param instr The instruction to await
/// @param timeout The timeout in milliseconds before returning back
/// 
/// @return Returns the node of the awaited instruction. Returns NULL if failed.
/// The node should be freed after using.
///
WIMP_API WimpInstrNode wimp_server_wait_response(WimpServer* server, uint64_t instr, int32_t timeout);

///
/// @brief Routes server instructions 
///
/// Routes server instructions to their destination if they aren't being sent to this server.
/// If instruction is successfully routed (returns true) do not try to free the instr node as 
/// ownership is passed to the outgoing queue.
/// 
/// @param server The server to route with
/// @param dest_process The destination process of the instruction
/// @param instrnode The node to route
/// 
/// @returns Returns true if the instruction was routed, otherwise false
///
WIMP_API bool wimp_server_instr_routed(WimpServer* server, const char* dest_process, WimpInstrNode instrnode);

///
/// @brief Sends the instructions in the outgoing queue
/// 
/// @param server The server to send instructions from
///
WIMP_API int32_t wimp_server_send_instructions(WimpServer* server);

///
/// @brief Checks if the parent process is alive
///
/// Can be used as a failsafe to make sure program exits if the parent is killed
/// early and cannot clean up and send the exit signal. Should call in
/// the main loop of a program that might be a separate executable.
/// 
/// @param server The server to check
/// 
/// @return Returns true if the parent is alive, or the process doesn't
/// have a parent. Otherwise returns false.
///
WIMP_API bool wimp_server_is_parent_alive(WimpServer* server);

///
/// @brief Frees the Wimp Server
/// 
/// @param server The server to free
///
WIMP_API void wimp_server_free(WimpServer* server);

#endif