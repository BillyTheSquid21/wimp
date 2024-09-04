///
/// @file
///
/// This header defines the interfaces to wimp instructions
///
/// An instruction is formatted as such:
/// 
/// TOTAL_BYTES-DESTPROCESS\0-SOURCEPROCESS\0-INSTRUCTION-ARG_BYTES-...
/// 
/// An individual instruction can only be up to WIMP_MESSAGE_BUFFER_BYTES long.
/// 
/// Each server has two linked lists for instructions (queue), one is incoming
/// and one is outgoing. Formatted as FIFO. The reciever(s) for the process
/// add to the incoming queue (as multiple threads could write need to use
/// mutexes) which the process reads and executes. In the process of executing,
/// outgoing instructions may be added from the same thread. Each thread usually
/// has one server, which is a thread local pointer. This means as long as a
/// server exists on the thread, including the server header calling the function to
/// add instructions is all that needs to be done. Once the instruction data is
/// not used anymore, must free the instruction node. Can create a specific instance
/// of a local server but this will need to be passed to any functions adding instrs.
/// 
/// Some design considerations to change depending on performance for different
/// applications:
///
/// - Nodes could share some memory space for faster reading of instructions,
///   potentially working like buckets of instructions up to a limit size.
///

#ifndef WIMP_INSTRUCTION_H
#define WIMP_INSTRUCTION_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <plibsys.h>
#include <assert.h>
#include <wimp_core.h>
#include <wimp_debug.h>

///
/// Instructions are hashed at compile time for fast lookup.
/// Non static version of the function (wimp_instr(...)) is provided to allow runtime matching.
/// The static hash function takes up to 32 characters.
///

#define WIMP_INSTRUCTION_MAX_LENGTH 32

#define WIMP_INSTR_HASH_BASIS 17ULL
#define WIMP_INSTR_HASH_PRIME 11ULL

#define W_HASH_1(ARG1) ((ARG1 ^ WIMP_INSTR_HASH_BASIS) * WIMP_INSTR_HASH_PRIME)
#define W_HASH_2(ARG1, ARG2) ((ARG2 ^ W_HASH_1(ARG1)) * WIMP_INSTR_HASH_PRIME)
#define W_HASH_3(ARG1, ARG2, ARG3) ((ARG3 ^ W_HASH_2(ARG1, ARG2)) * WIMP_INSTR_HASH_PRIME)
#define W_HASH_4(ARG1, ARG2, ARG3, ARG4)                                       \
    ((ARG4 ^ W_HASH_3(ARG1, ARG2, ARG3)) * WIMP_INSTR_HASH_PRIME)
#define HASH_5(ARG1, ARG2, ARG3, ARG4, ARG5)                                   \
    ((ARG5 ^ W_HASH_4(ARG1, ARG2, ARG3, ARG4)) * WIMP_INSTR_HASH_PRIME)
#define HASH_6(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)                             \
    ((ARG6 ^ HASH_5(ARG1, ARG2, ARG3, ARG4, ARG5)) * WIMP_INSTR_HASH_PRIME)
#define HASH_7(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)                       \
    ((ARG7 ^ HASH_6(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)) * WIMP_INSTR_HASH_PRIME)
#define HASH_8(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)                 \
    ((ARG8 ^ HASH_7(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)) * WIMP_INSTR_HASH_PRIME)
#define HASH_9(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)           \
    ((ARG9 ^ HASH_8(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)) * WIMP_INSTR_HASH_PRIME)
#define HASH_10(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10)   \
    ((ARG10 ^ HASH_9(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)) * WIMP_INSTR_HASH_PRIME)
#define HASH_11(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11) \
    ((ARG11 ^ HASH_10(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10)) * WIMP_INSTR_HASH_PRIME)
#define HASH_12(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12) \
    ((ARG12 ^ HASH_11(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11)) * WIMP_INSTR_HASH_PRIME)
#define HASH_13(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13) \
    ((ARG13 ^ HASH_12(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12)) * WIMP_INSTR_HASH_PRIME)
#define HASH_14(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14) \
    ((ARG14 ^ HASH_13(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13)) * WIMP_INSTR_HASH_PRIME)
#define HASH_15(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15) \
    ((ARG15 ^ HASH_14(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14)) * WIMP_INSTR_HASH_PRIME)
#define HASH_16(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16) \
    ((ARG16 ^ HASH_15(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15)) * WIMP_INSTR_HASH_PRIME)
#define HASH_17(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17) \
    ((ARG17 ^ HASH_16(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16)) * WIMP_INSTR_HASH_PRIME)
#define HASH_18(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18) \
    ((ARG18 ^ HASH_17(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17)) * WIMP_INSTR_HASH_PRIME)
#define HASH_19(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19) \
    ((ARG19 ^ HASH_18(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18)) * WIMP_INSTR_HASH_PRIME)
#define HASH_20(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20) \
    ((ARG20 ^ HASH_19(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19)) * WIMP_INSTR_HASH_PRIME)
#define HASH_21(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21) \
    ((ARG21 ^ HASH_20(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20)) * WIMP_INSTR_HASH_PRIME)
#define HASH_22(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22) \
    ((ARG22 ^ HASH_21(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21)) * WIMP_INSTR_HASH_PRIME)
#define HASH_23(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23) \
    ((ARG23 ^ HASH_22(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22)) * WIMP_INSTR_HASH_PRIME)
#define HASH_24(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24) \
    ((ARG24 ^ HASH_23(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23)) * WIMP_INSTR_HASH_PRIME)
#define HASH_25(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25) \
    ((ARG25 ^ HASH_24(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24)) * WIMP_INSTR_HASH_PRIME)
#define HASH_26(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26) \
    ((ARG26 ^ HASH_25(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25)) * WIMP_INSTR_HASH_PRIME)
#define HASH_27(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27) \
    ((ARG27 ^ HASH_26(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26)) * WIMP_INSTR_HASH_PRIME)
#define HASH_28(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28) \
    ((ARG28 ^ HASH_27(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27)) * WIMP_INSTR_HASH_PRIME)
#define HASH_29(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29) \
    ((ARG29 ^ HASH_28(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28)) * WIMP_INSTR_HASH_PRIME)
#define HASH_30(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29, ARG30) \
    ((ARG30 ^ HASH_29(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29)) * WIMP_INSTR_HASH_PRIME)
#define HASH_31(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29, ARG30, ARG31) \
    ((ARG31 ^ HASH_30(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29, ARG30)) * WIMP_INSTR_HASH_PRIME)
#define HASH_32(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29, ARG30, ARG31, ARG32) \
    ((ARG32 ^ HASH_31(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29, ARG30, ARG31)) * WIMP_INSTR_HASH_PRIME)

#define W_HASH_COUNT(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29, ARG30, ARG31, ARG32, func, ...)  \
    func

/// @brief Static hash function for up to 32 characters, allows using switch statements on instructions
#define WINSTR(...)                                                              \
    W_HASH_COUNT(__VA_ARGS__, HASH_32(__VA_ARGS__), HASH_31(__VA_ARGS__), HASH_30(__VA_ARGS__), \
               HASH_29(__VA_ARGS__), HASH_28(__VA_ARGS__), HASH_27(__VA_ARGS__), \
               HASH_26(__VA_ARGS__), HASH_25(__VA_ARGS__), HASH_24(__VA_ARGS__), \
               HASH_23(__VA_ARGS__), HASH_22(__VA_ARGS__), HASH_21(__VA_ARGS__), \
               HASH_20(__VA_ARGS__), HASH_19(__VA_ARGS__), HASH_18(__VA_ARGS__), \
               HASH_17(__VA_ARGS__), HASH_16(__VA_ARGS__), HASH_15(__VA_ARGS__), \
               HASH_14(__VA_ARGS__), HASH_13(__VA_ARGS__), HASH_12(__VA_ARGS__), \
               HASH_11(__VA_ARGS__), HASH_10(__VA_ARGS__), HASH_9(__VA_ARGS__), \
               HASH_8(__VA_ARGS__), HASH_7(__VA_ARGS__), HASH_6(__VA_ARGS__), \
               HASH_5(__VA_ARGS__), W_HASH_4(__VA_ARGS__), W_HASH_3(__VA_ARGS__), \
               W_HASH_2(__VA_ARGS__), W_HASH_1(__VA_ARGS__)) 



/// @brief The reserved core instructions for WIMP - Hashed at compile time
enum WIMPInstructionsCore
{
	WIMP_INSTRUCTION_EXIT             = WINSTR('e','x','i','t'),				///< Signals the target process and it's children to exit
	WIMP_INSTRUCTION_LOG              = WINSTR('l','o','g'),					///< Signals a logging instruction
	WIMP_INSTRUCTION_PING             = WINSTR('p','i','n','g'),				///< Used for checking a server is still listening
	WIMP_INSTRUCTION_HANDSHAKE_STATUS = WINSTR('h','_','s','t','a','t','u','s'),///< Used for determining if a server has correctly connected
};

#define WIMP_INSTRUCTION_DEST_OFFSET sizeof(int32_t)

/// @brief The result of a WIMP instruction operation
enum WimpInstructionResult
{
	WIMP_INSTRUCTION_SUCCESS = 0, ///< Result if instruction operation is successful
    WIMP_INSTRUCTION_FAIL    = -1 ///< Result if instruction operation fails for an unspecified reason
};

/// @brief Contains the data of an instruction
typedef struct _WimpInstr
{
	uint8_t* instruction;
	size_t instruction_bytes;
} WimpInstr;

/// @brief A node used in the instruction queues
typedef struct _WimpInstrNode *WimpInstrNode;

/// @brief Defines a linked list instruction queue
typedef struct _WimpInstrQueue
{
	WimpInstrNode nextnode; ///< The next node, if one exists
	WimpInstrNode backnode; ///< The end node, if one exists
	PMutex* _datamutex;
	PMutex* _nextmutex;
	PMutex* _lowpriomutex; //Uses the triple mutex pattern
} WimpInstrQueue;

#define WIMP_STR_PACK_MAX_STRINGS 8

///
/// @brief Contiguous pack of strings with automatic null termination
///
/// Provides an easy way to send multiple string arguments
/// - Strings are stored in the same memory chunk as the pack data for easy copying
/// - Strings are given as offsets to avoid pointer invalidation
///
typedef struct _WimpStrPack
{
	size_t pack_size;							///< The size of the total pack
	size_t str_count;							///< The number of strings in the pack
	size_t strings[WIMP_STR_PACK_MAX_STRINGS];	///< The offsets of each string, up to WIMP_STR_PACK_MAX_STRINGS length
} *WimpStrPack;

///
/// @brief Instruction metadata that can be pulled from a buffer
///
typedef struct _WimpInstrMeta
{
	const char* source_process;	///< Name of the source process
	const char* dest_process;	///< Name of the destination process
	uint64_t instr;			    ///< Instruction value
	void* args;					///< Pointer to the arguments
	size_t total_bytes;			///< Total size in bytes of the instruction
	int32_t arg_bytes;			///< Total size in bytes of the arguments only
	int32_t instr_bytes;		///< Total size in bytes of the instruction only
} WimpInstrMeta;

///
/// @brief The runtime equivalent of the wimp instruction hasher. 
/// Is not cryptographically secure, but will create a unique ID.
///
/// @param text The text instruction to hash
///
/// @return Returns the 64 bit hash of the instruction
///
uint64_t wimp_instr(const char *text);

///
/// Get the start of the raw instruction data 
///
#define WIMP_INSTR_START(meta) (uint8_t*)(meta.dest_process - WIMP_INSTRUCTION_DEST_OFFSET)

///
/// Get the offset into the raw instruction data
///
#define WIMP_INSTR_OFFSET(meta, offset) (uint8_t*)(meta.dest_process - WIMP_INSTRUCTION_DEST_OFFSET + offset)

///
/// @brief Creates a new instruction queue
/// 
/// @return Returns an instruction queue
///
WIMP_API WimpInstrQueue wimp_create_instr_queue(void);

///
/// @brief Performs low priority locking operations for the queue
///
/// Usually used in the reciever thread to give priority to the main process
/// 
/// @param queue The queue to lock
///
WIMP_API void wimp_instr_queue_low_prio_lock(WimpInstrQueue* queue);

///
/// @brief Performs low priority unlocking operations for the queue
/// 
/// @param queue The queue to unlock
///
WIMP_API void wimp_instr_queue_low_prio_unlock(WimpInstrQueue* queue);

///
/// @brief Performs high priority locking operations for the queue
///
/// Usually used in the main thread to ensure it has priority over the reciever threads
/// 
/// @param queue The queue to lock
///
WIMP_API void wimp_instr_queue_high_prio_lock(WimpInstrQueue* queue);

///
/// @brief Performs high priority unlocking operations for the queue
/// 
/// @param queue The queue to unlock
///
WIMP_API void wimp_instr_queue_high_prio_unlock(WimpInstrQueue* queue);

///
/// @brief Adds an instruction to the queue
/// 
/// @param queue The pointer to the queue to add to
/// @param instr A heap pointer to the instruction buffer, which will later be freed automatically
/// @param bytes The size of the instruction buffer in bytes
/// 
/// @return Returns either WIMP_INSTRUCTION_SUCCESS or WIMP_INSTRUCTION_FAIL
///
WIMP_API int32_t wimp_instr_queue_add(WimpInstrQueue* queue, void* instr, size_t bytes);

///
/// Adds an existing instruction node to the queue 
///
/// Implicitly passes ownership to the specified queue
/// 
/// @param queue The pointer to the queue to add to
/// @param node The node to give to the queue
/// 
/// @return Returns either WIMP_INSTRUCTION_SUCCESS or WIMP_INSTRUCTION_FAIL
///
WIMP_API int32_t wimp_instr_queue_add_existing(WimpInstrQueue* queue, WimpInstrNode node);

///
/// @brief Prepends an existing queue to the front of a queue
/// 
/// @param queue The queue being added to
/// @param add The queue to add
/// 
/// @return Returns either WIMP_INSTRUCTION_SUCCESS or WIMP_INSTRUCTION_FAIL
///
WIMP_API int32_t wimp_instr_queue_prepend_queue(WimpInstrQueue* queue, WimpInstrQueue* add);

///
/// @brief Appends an existing queue to the back of a queue
///
/// @param queue The queue being added to
/// @param add The queue to add
///
/// @return Returns either WIMP_INSTRUCTION_SUCCESS or WIMP_INSTRUCTION_FAIL
///
WIMP_API int32_t wimp_instr_queue_append_queue(WimpInstrQueue* queue, WimpInstrQueue* add);

///
/// @brief Pops the top node off the queue
/// 
/// When a node is returned, the user is responsible for its memory and it cannot
/// be accessed from the queue anymore. Use wimp_instr_node_free when done as
/// ownership is implicitly passed to the user
/// 
/// @param queue The queue to pop the top instruction off
/// 
/// @return Returns a pointer to the top node, NULL if queue is empty
///
WIMP_API WimpInstrNode wimp_instr_queue_pop(WimpInstrQueue* queue);

///
/// @brief Frees the memory used for the queue node
/// 
/// @param node The node pointer to free
///
WIMP_API void wimp_instr_node_free(WimpInstrNode node);

///
/// @brief Frees the memory used for the instruction queue
/// 
/// @param queue The queue to free
///
WIMP_API void wimp_instr_queue_free(WimpInstrQueue queue);

///
/// @brief Gets an instruction metadata from a buffer
///
/// Assumes buffer starts at start of an instruction
///
/// @param buffer The buffer to extract from
/// @param buffsize The buffer size to extract in bytes
///
/// @return Returns the metadata of the instruction
///
WIMP_API WimpInstrMeta wimp_instr_get_from_buffer(uint8_t* buffer, size_t buffsize);

///
/// @brief Extracts the wimp instruction metadata from a node.
///
/// @param node The node to extract from
///
/// @return Returns the metadata of the instruction
///
WIMP_API WimpInstrMeta wimp_instr_get_from_node(WimpInstrNode node);

///
/// @brief Compares two instructions to check if they're the same
/// 
/// @param instr1 The first instruction
/// @param instr2 The second instruction
/// 
/// @return Returns whether the instructions match or not
///
WIMP_API bool wimp_instr_check(uint64_t instr1, uint64_t instr2);

///
/// @brief Counts the amount of instructions with a given name in the queue
/// 
/// @param queue The queue to count the instructions in
/// 
/// @return Returns the instruction count
///
WIMP_API size_t wimp_instr_get_instruction_count(WimpInstrQueue* queue, const char* instruction);

///
/// @brief Packs a collection of up to WIMP_STR_PACK_MAX_STRINGS strings
/// 
/// @param count The number of strings
/// @param ... The strings to pack
/// 
/// @return Returns a pointer to the packed strings interface
///
WIMP_API WimpStrPack wimp_instr_pack_strings(size_t count, ...);

///
/// @brief Gets a string at a given index from the string pack
/// 
/// @param pack The pack to get the string from
/// @param index The index of the string
/// 
/// @return Returns the C style string pointer
///
WIMP_API char* wimp_instr_pack_get_string(WimpStrPack pack, int32_t index);

///
/// @brief Frees a string pack allocated with pack strings
/// 
/// This doesn't need to be called when a pack is retrieved from an instruction
/// 
/// @param pack A pointer to the pack to free
///
WIMP_API void wimp_instr_pack_free(WimpStrPack* pack);

#endif