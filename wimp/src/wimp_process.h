#ifndef WIMP_PROCESS_H
#define WIMP_PROCESS_H

#include <wimp_reciever.h>

#define WIMP_PROCESS_SUCCESS 0
#define WIMP_PROCESS_FAIL -1
#define MAX_PORT_STRING_LEN 6

/*
* Processes can be launched, and the recieve threads and the end processes are
* kept track of with a lookup table.
*/

typedef int (*MAIN_FUNC_PTR)(int, const char*); //Main function type pointer

typedef const char wimp_port_str[MAX_PORT_STRING_LEN]; //String representation of a port

/*
* Is basically a wrapper for the normal entry args so can be called from a
* thread.
*/
typedef struct _WimpMainEntry
{
	int argc;
	char** argv;
} *WimpMainEntry;

enum WIMP_ProcessType
{
	WIMP_ProcessType_Library = 0,	//Spins off a thread that uses a main() function from a library included in the master thread implementation
	WIMP_ProcessType_Executable = 1,//Spins off a thread that starts a separate executable
};

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
* Starts a library process
*/
int32_t wimp_start_library_process(const char* process_name, MAIN_FUNC_PTR main_func, WimpMainEntry entry);

/*
* Creates a persistent collection of arguments to supply.
* Will create a heap copy of all the data supplied.
*/
WimpMainEntry wimp_get_entry(int32_t argc, ...);

/*
* Gets an unused local port from the OS by binding a dummy socket
*/
int32_t wimp_assign_unused_local_port();

/*
* Converts the numerical port to a stack string representation
*/
int32_t wimp_port_to_string(int32_t port, wimp_port_str* string_out);

/*
* Frees the entry - must be done in the child thread
*/
void wimp_free_entry(WimpMainEntry entry);

#endif