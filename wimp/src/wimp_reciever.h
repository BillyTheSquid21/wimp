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

/*
* Struct containing the arguments for the reciever, with a target domain
* and port number.
*/
typedef struct _RecieverArgs
{
	const char* recfrom_domain;
	int32_t recfrom_port;
	const char* process_domain;
	int32_t process_port;
	uint8_t* writebuff;
} *RecieverArgs;

/*
* Creates a persistent collection of arguments to supply to the reciever thread.
* Will create a heap copy of all the data supplied which is freed at the end of
* the reciever thread.
*/
RecieverArgs wimp_get_reciever_args(const char* recfrom_domain, int32_t recfrom_port, const char* process_domain, int32_t process_port, uint8_t* wrtbuff);

/*
* Reciever loop
*/
void wimp_reciever_recieve(RecieverArgs args);

/*
* Starts a reciever thread
*/
int32_t wimp_start_reciever_thread(const char* reciever_name, RecieverArgs args);

#endif