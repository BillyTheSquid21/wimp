#ifndef WIMP_RECIEVER_H
#define WIMP_RECIEVER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <plibsys.h>
#include <pmacros.h>
#include <ptypes.h>
#include <perrortypes.h>

#define WIMP_RECIEVER_HANDSHAKE 0x706d6977 //HEX for "wimp"
#define WIMP_RECIEVER_SUCCESS 0
#define WIMP_RECIEVER_FAIL -1
#define WIMP_MESSAGE_BUFFER_BYTES 1024
#define WIMP_ZERO_BUFFER(buffer) memset(buffer, 0, WIMP_MESSAGE_BUFFER_BYTES)

typedef int (*RECIEVER_FUNC_PTR)(const char* int32_t); //Reciever function type pointer

typedef uint8_t WimpMsgBuffer[WIMP_MESSAGE_BUFFER_BYTES];

/*
* Struct containing the handshake header information
*/
typedef struct _WimpHandshakeHeader
{
	int32_t handshake_header;
	int32_t process_name_bytes;
	//Then followed by the name of the process

} WimpHandshakeHeader;

/*
* Struct containing the arguments for the reciever, with a target domain
* and port number.
*/
typedef struct _RecieverArgs
{
	const char* process_name;
	const char* recfrom_domain;
	int32_t recfrom_port;
	uint8_t* writebuff;
} *RecieverArgs;

/*
* Creates the handshake in the supplied message buffer and returns the header
*/
WimpHandshakeHeader wimp_create_handshake(const char* process_name, uint8_t* message_buffer);

/*
* Creates a persistent collection of arguments to supply to the reciever thread.
* Will create a heap copy of all the data supplied which is freed at the end of
* the reciever thread automatically.
*/
RecieverArgs wimp_get_reciever_args(const char* process_name, const char* recfrom_domain, int32_t recfrom_port, uint8_t* wrtbuff);

/*
* Reciever loop
*/
void wimp_reciever_recieve(RecieverArgs args);

/*
* Starts a reciever thread
*/
int32_t wimp_start_reciever_thread(const char* rec_procname, const char* process_domain, int32_t process_port, RecieverArgs args);

#endif