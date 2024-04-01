/**
 * @file
 *
 * This header defines the interfaces to the wimp_process
 */

#ifndef WIMP_PROCESS_H
#define WIMP_PROCESS_H

#include <wimp_core.h>
#include <wimp_reciever.h>

#define WIMP_PROCESS_SUCCESS 0
#define WIMP_PROCESS_FAIL -1
#define MAX_PORT_STRING_LEN 6

/*
* Processes can be launched, and the recieve threads and the end processes are
* kept track of with a lookup table.
*/

typedef int (*MAIN_FUNC_PTR)(int, const char*); //Main function type pointer

typedef char WimpPortStr[MAX_PORT_STRING_LEN]; //String representation of a port

/*
* Is a wrapper for the normal entry args so can be called from a
* thread.
*/
typedef struct _WimpMainEntry
{
	char** argv;
	int argc;
} *WimpMainEntry;

/*
* Defines privilege of a process so end processes can reject underprivileged
* calls. Other than root there is no specific meaning - higher is less privileged
*/
typedef enum
{
	WIMP_Privilege_Root = 0,	
	WIMP_Privilege_1 = 1,
	WIMP_Privilege_2 = 2,
	WIMP_Privilege_3 = 3,
	WIMP_Privilege_4 = 4,
	WIMP_Privilege_5 = 5,
	WIMP_Privilege_6 = 6,
} WIMP_ProcessPrivilege;

/*
* Starts the wimp library
*
* @return Returns either WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
*/
WIMP_API int32_t wimp_init();

/*
* Shuts down the wimp library
*/
WIMP_API void wimp_shutdown();

/*
* Starts a library process
* 
* @param process_name The name of the process to create
* @param main_func The pointer to the main function, which should be set up for being a library process not an executable one
* @param priority The puthread thread priority
* @param entry The entry arguments
* 
* @return Returns either WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
*/
WIMP_API int32_t wimp_start_library_process(const char* process_name, MAIN_FUNC_PTR main_func, enum PUThreadPriority_ priority, WimpMainEntry entry);

/*
* Starts an executable process. Currently must be located downstream
* of the location of the calling executable to avoid arbitrary
* execution. If needed, may create a specific function for launching
* arbitrary exe's, although this might be very unsafe.
* 
* For now cannot automatically set exe priority. The user could supply
* this from command line args and set normally.
* 
* @param process_name The name of the process to create
* @param executable The executable relative to the location the program binary is running from (e.g. bin directory)
* @param entry The entry arguments
* 
* @return Returns either WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
*/
WIMP_API int32_t wimp_start_executable_process(const char* process_name, const char* executable, WimpMainEntry entry);

/*
* Creates a persistent collection of arguments to supply, which can be freed after the library main thread is finished
* 
* @param argc The number of arguments supplied
* @param ... The arguments, which must be null terminated strings
* 
* @return Returns the entry arguments
*/
WIMP_API WimpMainEntry wimp_get_entry(int32_t argc, ...);

/*
* Frees the entry - must be done in the child thread
* 
* @param entry The entry arguments
*/
WIMP_API void wimp_free_entry(WimpMainEntry entry);

/*
* Gets an unused local port from the OS by binding a dummy socket
* 
* @return Returns the port number if successful, otherwise returns WIMP_PROCESS_FAIL
*/
WIMP_API int32_t wimp_assign_unused_local_port(void);

/*
* Converts the numerical port to a stack string representation
* 
* @param port The numerical representation of the port
* @param string_out The location to write the string to
* 
* @return Returns either WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
*/
WIMP_API int32_t wimp_port_to_string(int32_t port, WimpPortStr string_out);

#endif