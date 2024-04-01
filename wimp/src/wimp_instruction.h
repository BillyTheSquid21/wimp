/*
* @file
*
* This header defines the interfaces to wimp instructions
*
* An instruction is formatted as such:
* 
* TOTAL_BYTES-DESTPROCESS\0-SOURCEPROCESS\0-INSTRUCTION\0-ARG_BYTES-...
* 
* An individual instruction can only be up to WIMP_MESSAGE_BUFFER_BYTES long.
* 
* Each server has two linked lists for instructions (queue), one is incoming
* and one is outgoing. Formatted as FIFO. The reciever(s) for the process
* add to the incoming queue (as multiple threads could write need to use
* mutexes) which the process reads and executes. In the process of executing,
* outgoing instructions may be added from the same thread. Each thread can only
* have one server, which is a thread local pointer. This means as long as a
* server exists on the thread, including the server header calling the function to
* add instructions is all that needs to be done. Once the instruction data is
* not used anymore, must free the instruction node.
* 
* Some design considerations to change depending on performance for different
* applications:
* 
* - Formatting the instructions as strings with constantly reused identifiers
*   is not ideal. Can improve by converting the plain text process names and
*   instructions with a hashed value which would reduce memory footprint.
*   Hashed values would also be faster for instruction lookup at the other end.
*   Therefore, this will be changed in the future once the base system works.
*
* - Nodes could share some memory space for faster reading of instructions,
*   potentially working like buckets of instructions up to a limit size.
* 
* - A thread_local server pointer limits instructions to being added from the
*   server thread. This is a limit that could be changed.
* 
* - Currently all threads have equal priority when adding/popping - need to
*   ensure that the server/popping thread has priority, otherwise if too
*   many instructions are added the master thread may not be able to move on
* 
* These will be implemented once the simplistic approach works.
*/

#ifndef WIMP_INSTRUCTION_H
#define WIMP_INSTRUCTION_H

#include <stdint.h>
#include <string.h>
#include <plibsys.h>
#include <assert.h>
#include <wimp_core.h>
#include <wimp_debug.h>

#define WIMP_INSTRUCTION_SUCCESS 0
#define WIMP_INSTRUCTION_FAIL -1
#define WIMP_INSTRUCTION_EXIT "exit"
#define WIMP_INSTRUCTION_LOG "log"
#define WIMP_INSTRUCTION_PING "ping"
#define WIMP_INSTRUCTION_HANDSHAKE_STATUS "handshake_status"
#define WIMP_INSTRUCTION_DEST_OFFSET sizeof(int32_t)

typedef struct _WimpInstr
{
	uint8_t* instruction;
	size_t instruction_bytes;
} WimpInstr;

typedef struct _WimpInstrNode
{
	WimpInstr instr;
	struct _WimpInstrNode* nextnode;
} *WimpInstrNode;

typedef struct _WimpInstrQueue
{
	WimpInstrNode nextnode;
	WimpInstrNode backnode;
	PMutex* _datamutex;
	PMutex* _nextmutex;
	PMutex* _lowpriomutex; //Uses the triple mutex pattern
} WimpInstrQueue;

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

/*
* Creates a new instruction queue
* 
* @return Returns a new instruction queue
*/
WIMP_API WimpInstrQueue wimp_create_instr_queue(void);

/*
* Performs low priority locking operations for the queue
* 
* @param queue The queue to lock
*/
WIMP_API void wimp_instr_queue_low_prio_lock(WimpInstrQueue* queue);

/*
* Performs low priority unlocking operations for the queue
* 
* @param queue The queue to unlock
*/
WIMP_API void wimp_instr_queue_low_prio_unlock(WimpInstrQueue* queue);

/*
* Performs high priority locking operations for the queue
* 
* @param queue The queue to lock
*/
WIMP_API void wimp_instr_queue_high_prio_lock(WimpInstrQueue* queue);

/*
* Performs high priority unlocking operations for the queue
* 
* @param queue The queue to unlock
*/
WIMP_API void wimp_instr_queue_high_prio_unlock(WimpInstrQueue* queue);

/*
* Adds an instruction to the queue
* 
* @param queue The pointer to the queue to add to
* @param instr A heap pointer to the instruction buffer, which will later be freed automatically
* @param bytes The size of the instruction buffer in bytes
* 
* @return Returns either WIMP_INSTRUCTION_SUCCESS or WIMP_INSTRUCTIOn_FAIL
*/
WIMP_API int32_t wimp_instr_queue_add(WimpInstrQueue* queue, void* instr, size_t bytes);

/*
* Adds an existing instruction node to the queue - passes ownership
* 
* @param queue The pointer to the queue to add to
* @param node The node to give to the queue
* 
* @return Returns either WIMP_INSTRUCTION_SUCCESS or WIMP_INSTRUCTIOn_FAIL
*/
WIMP_API int32_t wimp_instr_queue_add_existing(WimpInstrQueue* queue, WimpInstrNode node);

/*
* Pops the top node off the queue, handing it to the user.
* 
* When a node is returned, the user is responsible for its memory and it cannot
* be accessed from the queue anymore. Use wimp_instr_node_free when done.
* 
* @param queue The queue to pop the top instruction off
* 
* @return Returns a pointer to the top node, NULL if queue is empty
*/
WIMP_API WimpInstrNode wimp_instr_queue_pop(WimpInstrQueue* queue);

/*
* Frees the memory used for the queue node
* 
* @param node The node pointer to free
*/
WIMP_API void wimp_instr_node_free(WimpInstrNode node);

/*
* Frees the memory used for the instruction queue
* 
* @param queue The queue to free
*/
WIMP_API void wimp_instr_queue_free(WimpInstrQueue queue);

/*
* Extracts the wimp instruction metadata from a buffer. Assumes buffer starts at start of an instruction
*
* @param buffer The buffer to extract from
*
* @return Returns the metadata of the instruction
*/
WIMP_API WimpInstrMeta wimp_instr_get_from_buffer(uint8_t* buffer, size_t buffsize);

/*
* Extracts the wimp instruction metadata from a node.
*
* @param node The node to extract from
*
* @return Returns the metadata of the instruction
*/
WIMP_API WimpInstrMeta wimp_instr_get_from_node(WimpInstrNode node);

#endif