///
/// @file
///
/// This header defines the interfaces to the wimp_reciever
///

#ifndef WIMP_RECIEVER_H
#define WIMP_RECIEVER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <utility/sds.h>

#include <plibsys.h>
#include <pmacros.h>
#include <ptypes.h>
#include <perrortypes.h>

#include <wimp_core.h>
#include <wimp_instruction.h>
#include <wimp_log.h>

#define WIMP_RECIEVER_HANDSHAKE 0x706d6977
#define WIMP_MESSAGE_BUFFER_BYTES 512
#define WIMP_RECIEVER_PING 0x676e6970
#define WIMP_ZERO_BUFFER(buffer) memset(buffer, 0, WIMP_MESSAGE_BUFFER_BYTES)
#define WIMP_PRINT_INSTRS 1 
#define WIMP_REC_TRY_INTERVAL 500 
#define WIMP_REC_TRY_COUNT 5

/// @brief The result of WIMP receiver operations
enum WimpRecieverResult
{
	WIMP_RECIEVER_SUCCESS 	= 0, ///< Result if reciever operation is successful
	WIMP_RECIEVER_FAIL 		= -1 ///< Result if reciever operation fails for an unspecified reason
};

///
/// @brief Reciever function type pointer
///
typedef int (*RECIEVER_FUNC_PTR)(const char* int32_t);

///
/// @brief A static buffer for WIMP packets to send/recieve from
///
/// Is of size WIMP_MESSAGE_BUFFER_BYTES.
///
typedef uint8_t WimpMsgBuffer[WIMP_MESSAGE_BUFFER_BYTES];

///
/// @brief Containins the handshake header information
/// 
/// @param handshake_header Header that should be equal to WIMP_RECIEVER_HANDSHAKE
/// @param process_name_bytes Length in bytes of the process name, in the buffer after the header struct. This is zero if the name overran the buffer.
///
typedef struct _WimpHandshakeHeader
{
	int32_t handshake_header;
	int32_t process_name_bytes;
} WimpHandshakeHeader;

///
/// @brief Reciver arguments structure
///
/// Contains the arguments for the reciever, with a target domain and port number.
///
typedef struct _RecieverArgs
{
	sds process_name;
	sds recfrom_domain;
	WimpInstrQueue* incoming_queue;
	int32_t recfrom_port;
	int32_t* active;
} *RecieverArgs;

#if defined _DEBUG && WIMP_PRINT_INSTRS

static void _debug_wimp_print_instruction_meta(WimpInstrMeta meta)
{
	printf("\nREAL INSTRUCTION FROM: %s\nTO: %s\nINSTR: %s\nARG SIZE: %d\nTOTAL SIZE: %d\n\n",meta.source_process,meta.dest_process,meta.instr,meta.arg_bytes,(int32_t)meta.total_bytes);
}

//Prints incoming instructions for debugging purposes
#define DEBUG_WIMP_PRINT_INSTRUCTION_META(meta) _debug_wimp_print_instruction_meta(meta)

#else

#define DEBUG_WIMP_PRINT_INSTRUCTION_META(meta)

#endif

///
/// @brief Gets the instruction size information
/// 
/// @param buffer The buffer to extract from
/// 
/// @return Returns the size of the instruction
///
WIMP_API int32_t wimp_get_instr_size(uint8_t* buffer);

///
/// @brief Creates a WIMP handshake
///
/// Creates in the supplied message buffer and returns the header
/// 
/// @param process_name The name of the process this reciever writes instructions to
/// @param message_buffer A pointer to the buffer to write the handshake into
/// 
/// @return Returns a copy of the header. This will be intialized to { 0, 0 } if function fails for any reason.
///
WIMP_API WimpHandshakeHeader wimp_create_handshake(const char* process_name, uint8_t* message_buffer);

///
/// @brief Creates the reciever arguments
///
/// Creates a persistent collection of arguments to supply to the reciever thread.
/// Will create a heap copy of all the data supplied which is freed at the end of
/// the reciever thread automatically.
///
/// @param process_name The name of the process this reciever writes instructions to
/// @param recfrom_domain The domain of the process this reciever will recieve from
/// @param recfrom_port The port of the process this reciever will reciever from
/// @param incomingq The queue to add the incoming instructions to
/// @param active An int that signals whether the recieving server is active
/// 
/// @return Returns the arguments generated
///
WIMP_API RecieverArgs wimp_get_reciever_args(const char* process_name, const char* recfrom_domain, int32_t recfrom_port, WimpInstrQueue* incomingq, int32_t* active);

///
/// @brief Starts a reciever thread
/// 
/// @param recfrom_name The name of the process to recieve from. Used to create the unique thread name for the reciever thread.
/// @param process_domain The domain that this reciver writes to
/// @param process_port The port that this reciever writes to
/// @param args The arguments to pass to the reciever
/// 
/// @return Returns either WIMP_RECIEVER_SUCCESS or WIMP_RECIEVER_FAIL
///
WIMP_API int32_t wimp_start_reciever_thread(const char* recfrom_name, const char* process_domain, int32_t process_port, RecieverArgs args);

#endif