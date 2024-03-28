#include <utility/simple_arena.h>

bool sarena_init_new(SArena* arena, size_t capacity)
{
	uint8_t* data = malloc(capacity);
	if (data == NULL)
	{
		return false;
	}

	bool res = sarena_init(arena, data, capacity);
	if (!res)
	{
		free(data);
	}

	return res;
}

bool sarena_init(SArena* arena, uint8_t* ptr, size_t capacity)
{
	if (arena->_arena_capacity || arena->_data)
	{
		return false;
	}

	arena->_arena_capacity = capacity;
	arena->_arena_size = 0;
	arena->_data = ptr;
	return true;
}

void sarena_free(SArena* arena)
{
	arena->_arena_capacity = 0;
	arena->_arena_size = 0;
	free(arena->_data);
	arena->_data = NULL;
	return;
}

SArenaPtr sarena_alloc(SArena* arena, size_t size)
{
	size_t new_arena_ptr = arena->_arena_pointer + size;
	if (new_arena_ptr >= arena->_arena_capacity)
	{
		return NULL;
	}

	size_t old_arena_ptr = arena->_arena_pointer;
	arena->_arena_pointer = new_arena_ptr;
	arena->_arena_size += size;
	return old_arena_ptr;
}