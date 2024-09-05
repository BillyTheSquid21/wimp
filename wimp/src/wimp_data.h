#ifndef WIMP_DATA_H
#define WIMP_DATA_H

#include <stdint.h>
#include <plibsys.h>
#include <wimp_core.h>
#include <wimp_log.h>
#include <utility/HashString.h>
#include <utility/sds.h>
#include <utility/simple_arena.h>

#define WIMP_MAX_SHARED_SLOTS 64
#define WIMP_SHARED_SLOT_MAX_NAME_BYTES 64

///
/// The data table will work by having multiple shared memory segments, which
/// are created/destroyed on master start and finish:
/// - A shared segment for the table, which controls access between the
///   processes and has a maximum number of entries
/// - Shared segments for each data being stored
/// 
/// Because of access from many process types, raw pointers should not be used
/// between processes, however area pointers relative to the start position
/// are portable.
/// 
/// Slots contain a capacity, the size currently used, and the name of the
/// shared segment. 
///

/// @brief The result of WIMP data operations
enum WimpDataResult
{
    WIMP_DATA_SUCCESS = 0, ///< Result if data operation is successful
    WIMP_DATA_FAIL    = -1 ///< Result if data operation fails for an unspecified reason
};

/// @brief Defines a slot containing data in shared memory
typedef struct _WimpDataSlot
{
    int32_t share_counter;
    size_t size;
    SArena data_arena;
    char name[WIMP_SHARED_SLOT_MAX_NAME_BYTES];
} WimpDataSlot;

///
/// @brief A shared memory arena allocator structure
///
/// Allows arena like allocations, and vector like indexing.
///
typedef struct _WimpDataArena
{
    SArena _arena;
    PShm* _shm;
} WimpDataArena;

/// @brief A relative pointer within a data arena
typedef SArenaPtr WArenaPtr;

///
/// @brief Convert the arena pointer into a normal pointer
///
/// New pointer is invalid outside the program space the arena was intialized in.
/// 
/// @param arena The arena to get the absolute pointer from
/// @param aptr The relative arena pointer to convert
///
#define WIMP_ARENA_GET_PTR(arena, aptr) SARENA_GET_PTR(arena._arena, aptr)

///
/// @brief Dereferences the arena pointer
///
/// New pointer is invalid outside the program space the arena was intialized in.
/// 
/// @param arena The arena to get the absolute pointer from
/// @param aptr The relative arena pointer to dereference
///
#define WIMP_ARENA_DEREF_APTR(arena, aptr) SARENA_DEREF_APTR(arena._arena, aptr)

///
/// @brief Indexes into the arena like an array
///
/// Is the same as DEREF_APTR
/// 
/// @param arena The arena to get the absolute pointer from
/// @param aptr The relative arena pointer to dereference
///
#define WIMP_ARENA_INDEX(arena, index) SARENA_DEREF_APTR(arena._arena, index)

///
/// @brief Gets the size of an arena as a const size_t
/// 
/// @param arena The arena to get the size of
///
#define WIMP_ARENA_SIZE(arena) ((const size_t)(arena._arena._arena_size))

///
/// @brief Gets the capacity of an arena as a const size_t
/// 
/// @param arena The arena to get the capacity of
///
#define WIMP_ARENA_CAPACITY(arena) ((const size_t)(arena._arena._arena_capacity))

///
/// @brief Allocates to the arena and increases the alloc pointer
///
/// @param arena A pointer to the arena to alloc to
/// @param How much space to allocate
///
/// @return Returns an arena pointer to a location in the arena.
///
#define WIMP_ARENA_ALLOC(arena, size) (WArenaPtr)sarena_alloc(&arena._arena, size);

///
/// @brief Initializes the WIMP data functionality
///
/// Initializes the data table, which creates a shared memory space for multiple processes. 
/// Called once by the master process. Pair with wimp_data_free at the end of the program.
/// 
/// @param memory_name The name of the program (and of the shared memory)
/// 
/// @return Returns WIMP_DATA_SUCCESS or WIMP_DATA_FAIL
///
WIMP_API int32_t wimp_data_init(const char* memory_name);

///
/// @brief Links the current process to the global data table
///
/// Should not call from the master process that calls wimp_data_init
/// 
/// @param memory_name The name of the program (and of the shared memory)
/// 
/// @return Returns WIMP_DATA_SUCCESS or WIMP_DATA_FAIL
///
WIMP_API int32_t wimp_data_link_to_process(const char* memory_name);

///
/// @brief Unlinks from the process. 
///
/// Should not call from the master process that calls wimp_data_init
///
WIMP_API void wimp_data_unlink_from_process(void);

///
/// @brief Frees the data table and takes ownership of the memory back.
///
WIMP_API void wimp_data_free(void);

///
/// @brief Reserves a space in memory to copy or write to
/// 
/// @param reserved_name The name for the memory segement to create
/// @param size The size to reserve.
/// 
/// @return Returns WIMP_DATA_SUCCESS or WIMP_DATA_FAIL
///
WIMP_API int32_t wimp_data_reserve(const char* reserved_name, size_t size);

///
/// @brief Links to the shared data and increases the share counter
/// 
/// @param name The name of the data to link to
/// 
/// @return Returns WIMP_DATA_SUCCESS or WIMP_DATA_FAIL
///
WIMP_API int32_t wimp_data_link_to_data(const char* name);

///
/// @brief Unlinks from the shared data and decreases the share counter
///
/// If no processes are still linked to the data, it will be returned
/// to the OS.
///
/// @param name The name of the data to unlink from
///
WIMP_API void wimp_data_unlink_from_data(const char* name);

///
/// @brief Get the arena of a location of data for accessing
///
/// Accessing locks the data and must be accompanied by a call to wimp_data_stop_access. 
/// Should wrap the access inside an if statement checking for success, otherwise will 
/// have locking fail.
/// 
/// @param arena A pointer to the arena pointer returned by the access call.
/// @param name The name of the shared data to access
/// 
/// @return Returns a pointer to the arena controlling the data TODO TYPEDEF
///
WIMP_API int32_t wimp_data_access(WimpDataArena* arena, const char* name);

///
/// @brief Ends accessing the data
/// 
/// Dereferences the arena ptr to ensure can't be used after.
///
/// @param arena A pointer to the arena pointer returned by the access call.
/// @param name The name of the shared data to stop accessing
///
WIMP_API void wimp_data_stop_access(WimpDataArena* arena, const char* name);

#endif