#ifndef WIMP_DATA_H
#define WIMP_DATA_H

#include <stdint.h>
#include <plibsys.h>
#include <wimp_log.h>
#include <utility/HashString.h>
#include <utility/sds.h>
#include <utility/simple_arena.h>

#define WIMP_DATA_SUCCESS 0
#define WIMP_DATA_FAIL -1
#define WIMP_MAX_SHARED_SLOTS 64
#define WIMP_SHARED_SLOT_MAX_NAME_BYTES 64

/*
* The data table will work by having multiple shared memory segments, which
* are created/destroyed on master start and finish:
* - A shared segment for the table, which controls access between the
*   processes and has a maximum number of entries
* - Shared segments for each data being stored
* 
* Because of access from many processes, pointers that are not within the
* shared spaces can NOT be used in API calls, as this will not be portable
* between the processes.
* 
* Slots contain a capacity, the size currently used, and the name of the
* shared segment. 
*/

typedef struct _WimpDataSlot
{
	SArena data_arena;
	bool active;
	size_t size;
	char name[WIMP_SHARED_SLOT_MAX_NAME_BYTES];
} WimpDataSlot;

/*
* The structure for data access which allows arena like allocations, and
* vector like indexing.
*/
typedef struct _WimpDataArena
{
	SArena* _arena;
	PShm* _shm;
} *WimpDataArena;

typedef SArenaPtr WArenaPtr;

/*
* Convert the arena pointer into a normal pointer. Invalid outside the
* program space the arena was intialized in.
*/
#define WIMP_ARENA_GET_PTR(arena, aptr) SARENA_GET_PTR(*arena->_arena, aptr)

/*
* Dereferences the arena pointer. Invalid outside the program space the
* arena was intialized in.
*/
#define WIMP_ARENA_DEREF_APTR(arena, aptr) SARENA_DEREF_APTR(*arena->_arena, aptr)

/*
* Indexes into the arena like an array. Is the same as DEREF_APTR
*/
#define WIMP_ARENA_INDEX(arena, index) SARENA_DEREF_APTR(*arena->_arena, index)

/*
* Gets the size of the arena as a const size_t
*/
#define WIMP_ARENA_SIZE(arena) ((const size_t)(arena->_arena->_arena_size))

/*
* Gets the capacity of the arena as a const size_t
*/
#define WIMP_ARENA_CAPACITY(arena) ((const size_t)(arena->_arena->_arena_capacity))

/*
* Allocates to the area and increases the alloc pointer
*
* @param arena A pointer to the arena to alloc to
* @param How much space to allocate
*
* @return Returns an arena pointer to a location in the arena.
*/
#define WIMP_ARENA_ALLOC(arena, size) (WArenaPtr)sarena_alloc(arena->_arena, size);

/*
* Initializes the data table, which creates a shared memory space for
* multiple processes. Can be used by any program regardless of if is
* connected based on name so bear in mind. Called once by the master
* process.
* 
* @param memory_name The name of the program (and of the shared memory)
* 
* @return Returns WIMP_DATA_SUCCESS or WIMP_DATA_FAIL
*/
int32_t wimp_data_init(const char* memory_name);

/*
* Links the current process to the data.
* 
* @param memory_name The name of the program (and of the shared memory)
* 
* @return Returns WIMP_DATA_SUCCESS or WIMP_DATA_FAIL
*/
int32_t wimp_data_link_to_process(const char* memory_name);

/*
* Unlinks from the process
*/
void wimp_data_unlink_from_process();

/*
* Frees the data table and takes ownership of the memory back.
* 
* @param memory_name The name of the program (and of the shared memory)
*/
void wimp_data_free(const char* memory_name);

/*
* Reserves a space in memory to copy or write to
*/
int32_t wimp_data_reserve(const char* reserved_name, size_t size);

/*
* Get the arena of a location of data for accessing. Accessing locks the
* data (hence why it can change the arena data pointer) and must be
* accompanied by a stop accessing call.
* 
* @param name The name of the shared data to access
* 
* @return Returns a pointer to the arena controlling the data TODO TYPEDEF
*/
SArena* wimp_data_access(const char* name);

/*
* Ends the accessing of the data. Dereferences the arena ptr to ensure can't
* be used after.
* 
* @param name The name of the shared data to stop accessing
* @param arena A pointer to the arena pointer returned by the access call.
*/
void wimp_data_stop_access(const char* name, WimpDataArena* arena);

#endif