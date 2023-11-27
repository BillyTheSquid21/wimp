#ifndef WIMP_RECIEVER_H
#define WIMP_RECIEVER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <plibsys.h>
#include <pmacros.h>
#include <ptypes.h>
#include <perrortypes.h>

#define WIMP_RECIEVER_HANDSHAKE 0x77696d70 //HEX for "wimp"
#define WIMP_RECIEVER_HANDSHAKE_LENGTH 4
#define WIMP_RECIEVER_SUCCESS 0
#define WIMP_RECIEVER_FAIL -1
#define WIMP_RECIEVER_MESSAGE_BUFFER_SIZE 512

typedef int (*RECIEVER_FUNC_PTR)(const char* int32_t); //Reciever function type pointer

typedef struct
{
	const char* domain;
	int32_t port_number;
	uint8_t* writebuff;
} RecieverArgs;

/*
* A WIMP reciever is the thread that recieves instructions from the end process
* and writes them to the instruction queue, so the master thread can avoid
* blocking. The master thread sends instructions when it needs to and does not
* block at all. The master sends instructions using a thread pool
* 
* The code that actually spins up the process thread is in wimp_process.h
* 
* For now just use TCP for connection for instructions. Uses plibsys threads.
*/

/*
* The simple loop carried out by the receiver thread. Does not start the end
* process, as that is handled by the master thread, or may already be running.
* 
* As ran on other thread, pass void pointer to a struct containing:
*	const char* domain, int32_t port_number, uint8_t* writebuff
*/
void wimp_reciever_recieve(RecieverArgs* args);









//Check if a command matches the recieved buffer - TODO probably put in a different header
bool check_buffer_command(const char* buffer, const char* command, size_t buffer_len, size_t command_len);
#endif