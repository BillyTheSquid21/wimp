#include <utility/simple_arena.h>

bool sarena_init(SArena* arena, size_t capacity)
{
	if (arena->arena_capacity || arena->_data)
	{
		return false;
	}

	//TODO - consider using mmap to make more dynamic region mapping
	void* data = malloc(capacity);
	if (data == NULL)
	{
		return false;
	}

	arena->arena_capacity = capacity;
	arena->arena_size = 0;
	arena->_data = data;
	return true;
}

void sarena_free(SArena* arena)
{
	arena->arena_capacity = 0;
	arena->arena_size = 0;
	free(arena->_data);
	arena->_data = NULL;
	return;
}

void* sarena_alloc(SArena* arena, size_t size)
{
	size_t new_arena_ptr = arena->_arena_pointer + size;
	if (new_arena_ptr >= arena->arena_capacity)
	{
		return NULL;
	}

	arena->_arena_pointer = new_arena_ptr;
	arena->arena_size += size; //Technically the same now but could change
	return (void*)(arena->_data + arena->_arena_pointer);
}