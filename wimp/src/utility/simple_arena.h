#ifndef SIMPLE_ARENA_H
#define SIMPLE_ARENA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/*
* A simple arena allocator
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
* - Dynamic growth allowed with mmap
*/

/*
* A simple arena struct that contains info about the size, capacity and data
*/
typedef struct _SArena
{
	size_t arena_size;
	size_t arena_capacity;
	size_t _arena_pointer;
	uint8_t* _data;
} SArena;

/*
* Defines the arena pointer which adds to the data pointer to allow passing
* "pointers" around regardless of where the data is - is not the default 
* returned - allows resizing without invalidating arena_ptrs. Is not always
* a concern, but may be desired behavior.
*/
typedef size_t arena_ptr;

#define APTR_TO_PTR(arena, aptr) (void*)((size_t)arena._data + aptr)
#define PTR_TO_APTR(arena, ptr) (arena_ptr)(ptr - arena._data)
#define DEREF_APTR(arena, aptr) *(arena._data + aptr)
#define INDEX_APTR(arena, aptr, i) *(arena._data + aptr + i)

/*
* Initialized the area and allocates the memory for the capacity
* 
* @param arena A pointer to the arena to initialize
* @param capacity How much capacity the arena should have
* 
* @return Returns whether the initialization was successful or not
*/
bool sarena_init(SArena* arena, size_t capacity);

/*
* Frees the arena and all the memory inside it
*/
void sarena_free(SArena* arena);

/*
* Allocates to the area and increases the alloc pointer
* 
* @param arena A pointer to the arena to alloc to
* @param How much space to allocate
* 
* @return Returns the pointer to the allocated section
* Returns NULL in case of failure
*/
void* sarena_alloc(SArena* arena, size_t size);

#endif