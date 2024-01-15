/**
 * @file
 *
 * This header defines the interfaces to the wimp_reciever
 */

#ifndef WIMP_RECIEVER_H
#define WIMP_RECIEVER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <plibsys.h>
#include <pmacros.h>
#include <ptypes.h>
#include <perrortypes.h>

#include <wimp_instruction.h>
#include <wimp_log.h>

#define WIMP_RECIEVER_HANDSHAKE 0x706d6977 //HEX for "wimp"
#define WIMP_RECIEVER_SUCCESS 0
#define WIMP_RECIEVER_FAIL -1
#define WIMP_MESSAGE_BUFFER_BYTES 1024
#define WIMP_RECIEVER_PING 0x00000000 //Ping with 4 bytes of null, ignore if this comes in
#define WIMP_ZERO_BUFFER(buffer) memset(buffer, 0, WIMP_MESSAGE_BUFFER_BYTES)
#define WIMP_PRINT_INSTRS 1 //Print the incoming instructions

/*
* Reciever function type pointer
*/
typedef int (*RECIEVER_FUNC_PTR)(const char* int32_t);

/*
* A static buffer for the network packets to be recieved/sent from. Is of size WIMP_MESSAGE_BUFFER_BYTES.
*/
typedef uint8_t WimpMsgBuffer[WIMP_MESSAGE_BUFFER_BYTES];

/*
* Struct containing the handshake header information
* 
* @param handshake_header Header that should be equal to WIMP_RECIEVER_HANDSHAKE
* @param process_name_bytes Length in bytes of the process name, in the buffer after the header struct. This is zero if the name overran the buffer.
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
	char* process_name;
	char* recfrom_domain;
	WimpInstrQueue* incoming_queue;
	int32_t recfrom_port;
} *RecieverArgs;

/*
* Instruction metadata that can be pulled from the buffer
*/
typedef struct _WimpInstrMeta
{
	const char* source_process;
	const char* dest_process;
	const char* instr;
	void* args;
	size_t total_bytes;
	int32_t arg_bytes;
	int32_t instr_bytes;
} WimpInstrMeta;

#if defined _DEBUG && WIMP_PRINT_INSTRS

static void _debug_wimp_print_instruction_meta(WimpInstrMeta meta)
{
	printf("\nREAL INSTRUCTION FROM: %s\n", meta.source_process);
	printf("TO: %s\n", meta.dest_process);
	printf("INSTR: %s\n", meta.instr);
	printf("ARG SIZE: %d\n", meta.arg_bytes);
	printf("TOTAL SIZE: %d\n\n", (int32_t)meta.total_bytes);
}

//Prints incoming instructions for debugging purposes
#define DEBUG_WIMP_PRINT_INSTRUCTION_META(meta) _debug_wimp_print_instruction_meta(meta)

#else

#define DEBUG_WIMP_PRINT_INSTRUCTION_META(meta)

#endif

/*
* Extracts the wimp instruction metadata from a buffer. Assumes buffer starts at start of an instruction
* 
* @param buffer The buffer to extract from
* 
* @return Returns the metadata of the instruction
*/
WimpInstrMeta wimp_get_instr_from_buffer(uint8_t* buffer, size_t buffsize);

/*
* Gets the instruction size header
* 
* @param buffer The buffer to extract from
* 
* @return Returns the size of the instruction
*/
int32_t wimp_get_instr_size(uint8_t* buffer);

/*
* Creates the handshake in the supplied message buffer and returns the header
* 
* @param process_name The name of the process this reciever writes instructions to
* @param message_buffer A pointer to the buffer to write the handshake into
* 
* @return Returns a copy of the header. This will be intialized to { 0, 0 } if function fails for any reason.
*/
WimpHandshakeHeader wimp_create_handshake(const char* process_name, uint8_t* message_buffer);

/*
* Creates a persistent collection of arguments to supply to the reciever thread.
* Will create a heap copy of all the data supplied which is freed at the end of
* the reciever thread automatically.
* 
* @param process_name The name of the process this reciever writes instructions to
* @param recfrom_domain The domain of the process this reciever will recieve from
* @param recfrom_port The port of the process this reciever will reciever from
* @param incomingq The queue to add the incoming instructions to
* 
* @return Returns the arguments generated
*/
RecieverArgs wimp_get_reciever_args(const char* process_name, const char* recfrom_domain, int32_t recfrom_port, WimpInstrQueue* incomingq);

/*
* Starts a reciever thread
* 
* @param recfrom_name The name of the process to recieve from. Used to create the unique thread name for the reciever thread.
* @param process_domain The domain that this reciver writes to
* @param process_port The port that this reciever writes to
* @param args The arguments to pass to the reciever
* 
* @return Returns either WIMP_RECIEVER_SUCCESS or WIMP_RECIEVER_FAIL
*/
int32_t wimp_start_reciever_thread(const char* recfrom_name, const char* process_domain, int32_t process_port, RecieverArgs args);

#endif