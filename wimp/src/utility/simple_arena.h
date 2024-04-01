#ifndef SIMPLE_ARENA_H
#define SIMPLE_ARENA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cassert>
#include <wimp_core.h>

/*
* A simple arena allocator. Packaged with wimp
* 
* Current features:
* - Contiguous Memory
*
* Potential future features: 
* - Debug cast system to allow changing the type of behavior
*   - Could set to act like std::vector in C++
*   - Could be set to act like a tree/network
*   - Would have functions used for those data structures, that when in debug
*     would check the type of arena
*/

/*
* A simple arena struct that contains info about the size, capacity and data
* Exposed to the user for freedom but not recommended to mess with the data.
*/
typedef struct _SArena
{
	size_t _arena_size;
	size_t _arena_capacity;
	size_t _arena_pointer;
	uint8_t* _data;
} SArena;

/*
* Arena pointer which points to the offset in the arena.
* 
* Can be converted to a standard pointer if the data pointer is valid.
* This may not always be the case (e.g. using shared memory), hence why
* more control is given.
*/
typedef size_t SArenaPtr;

/*
* Convert the arena pointer into a normal pointer. Invalid outside the
* program space the arena was intialized in.
*/
#define SARENA_GET_PTR(arena, aptr) (uint8_t*)((arena)._data + aptr)

/*
* Dereferences the arena pointer. Invalid outside the program space the
* arena was intialized in.
*/
#define SARENA_DEREF_APTR(arena, aptr) *((arena)._data + aptr)

/*
* Indexes into the arena like an array. Is the same as DEREF_APTR
*/
#define SARENA_INDEX(arena, index) SARENA_DEREF_APTR(arena, index)

/*
* Gets the size of the arena as a const size_t
*/
#define SARENA_SIZE(arena) ((const size_t)(arena._arena_size))

/*
* Gets the capacity of the arena as a const size_t
*/
#define SARENA_CAPACITY(arena) ((const size_t)(arena._arena_capacity))

/*
* Initialized the area and allocates the memory for the capacity
* 
* @param arena A pointer to the arena to initialize
* @param capacity How much capacity the arena should have
* 
* @return Returns whether the initialization was successful or not
*/
WIMP_API bool sarena_init_new(SArena* arena, size_t capacity);

/*
* Initialized the area from an existing memory segment (of size capacity)
*
* @param arena A pointer to the arena to initialize
* @param ptr A pointer to the allocated space for the arena
* @param capacity How much capacity the arena should have
*
* @return Returns whether the initialization was successful or not
*/
WIMP_API bool sarena_init(SArena* arena, uint8_t* ptr, size_t capacity);

/*
* Creates an empty sarena
* 
* @return Returns an empty arena
*/
WIMP_API SArena sarena();

/*
* Frees the arena and all the memory inside it. Calls free() on the data
* so can only be called on an arena where the memory was allocated with
* malloc() - otherwise will fail!
* 
* @param arena A pointer to the arena to free
*/
WIMP_API void sarena_free(SArena* arena);

/*
* Allocates to the area and increases the alloc pointer
* 
* @param arena A pointer to the arena to alloc to
* @param How much space to allocate
* 
* @return Returns an arena pointer to a location in the arena.
*/
WIMP_API SArenaPtr sarena_alloc(SArena* arena, size_t size);

#endif