#ifndef WIMP_PROCESS_H
#define WIMP_PROCESS_H

#include <wimp_reciever.h>

#define WIMP_PROCESS_SUCCESS 0
#define WIMP_PROCESS_FAIL -1

/*
* Processes can be launched, and the recieve threads and the end processes are
* kept track of with a lookup table.
*/

typedef int (*MAIN_FUNC_PTR)(int, const char*); //Main function type pointer

enum WIMP_ProcessType
{
	WIMP_ProcessType_Library = 0,	//Spins off a thread that uses a main() function from a library included in the master thread implementation
	WIMP_ProcessType_Executable = 1,//Spins off a thread that starts a separate executable
};

int32_t wimp_start_library_process(const char* process_name, MAIN_FUNC_PTR main_func, const char* domain, int32_t port_number, uint8_t* writebuff);
#endif