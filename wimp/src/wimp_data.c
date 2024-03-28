#include <wimp_data.h>
#include <utility/thread_local.h>

static thread_local PShm* s_DataTable = NULL;
static thread_local char* s_MemoryName = NULL;
static thread_local bool s_IsTableLinked = false;

#define TABLE_LENGTH sizeof(WimpDataSlot) * WIMP_MAX_SHARED_SLOTS

//The local arena instance
static thread_local struct _WimpDataArena s_Arena;

int32_t wimp_data_init(const char* memory_name)
{
	//Create a shared memory segment for the table
	PError* err = NULL;
	PShm* shm = p_shm_new(
		memory_name, 
		TABLE_LENGTH,
		P_SHM_ACCESS_READWRITE, 
		&err
	);

	psize size = p_shm_get_size(shm);

	if (err != NULL || size < TABLE_LENGTH)
	{
		wimp_log_fail("Failed to init data table: %s %d\n", p_error_get_message(err), p_error_get_code(err));
		p_shm_take_ownership(shm);
		p_shm_free(shm);
		return WIMP_DATA_FAIL;
	}

	//If all is well, get the pointer and init the slots
	WimpDataSlot* table_data = (WimpDataSlot*)p_shm_get_address(shm);
	for (size_t i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
	{
		table_data[i].active = false;
		memset(&table_data[i].name[0], 0, WIMP_SHARED_SLOT_MAX_NAME_BYTES);
	}

	//Init the local arena to nullptrs
	s_Arena._arena = NULL;
	s_Arena._shm = NULL;

	wimp_log_success("Created shared data table for program! %p\n", table_data);
	return WIMP_DATA_SUCCESS;
}

int32_t wimp_data_link_to_process(const char* memory_name)
{
	//Get the shared memory segment for the table
	PError* err = NULL;
	PShm* shm = p_shm_new(
		memory_name,
		TABLE_LENGTH,
		P_SHM_ACCESS_READWRITE,
		&err
	);

	if (err != NULL)
	{
		wimp_log_fail("Failed to link to data table: %s %d\n", p_error_get_message(err), p_error_get_code(err));
		p_shm_free(shm);
		return WIMP_DATA_FAIL;
	}

	s_DataTable = shm;
	s_MemoryName = memory_name;
	s_IsTableLinked = true;
	wimp_log_success("Successfully linked to the data table\n");
	return WIMP_DATA_SUCCESS;
}

void wimp_data_unlink_from_process()
{
	if (s_IsTableLinked)
	{
		p_shm_free(s_DataTable);
		s_IsTableLinked = false;
	}
	return;
}

void wimp_data_free(const char* memory_name)
{
	//TODO - Free child data then table
}

int32_t wimp_data_reserve(const char* reserved_name, size_t size)
{
	//Get the memory ptr
	WimpDataSlot* data_table = p_shm_get_address(s_DataTable);

	//Find a free slot
	for (int i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
	{
		if (!data_table[i].active)
		{
			//Copy the name
			memcpy(
				&data_table[i].name[0], 
				reserved_name, 
				(strlen(reserved_name) + 1) * sizeof(char)
			);

			//Create the shared memory
			PError* err = NULL;
			PShm* shm = p_shm_new(
				reserved_name,
				size,
				P_SHM_ACCESS_READWRITE,
				&err
			);

			psize shmsize = p_shm_get_size(shm);

			if (err != NULL || shmsize < size)
			{
				wimp_log_fail("Failed to allocate data: %s %d\n", p_error_get_message(err), p_error_get_code(err));
				p_shm_take_ownership(shm);
				p_shm_free(shm);
				return WIMP_DATA_FAIL;
			}

			//Add the pointer to the arena
			sarena_init(&data_table[i].data_arena, p_shm_get_address(shm), size);

			//Zero the data
			memset(data_table[i].data_arena._data, 0, size);

			//Free the shm struct and set active
			p_shm_free(shm);
			data_table[i].active = true;
			data_table[i].size = size;
			return WIMP_DATA_SUCCESS;
		}
	}

	return WIMP_DATA_FAIL;
}

SArena* wimp_data_access(const char* name)
{
	//Find the slot if it exists
	WimpDataSlot* data_table = p_shm_get_address(s_DataTable);
	for (int i = 0; i < TABLE_LENGTH; ++i)
	{
		if (strcmp(name, data_table[i].name) == 0)
		{
			//Get the shm pointer
			PError* err = NULL;
			PShm* shm = p_shm_new(
				name,
				data_table[i].size,
				P_SHM_ACCESS_READWRITE,
				&err
			);

			psize size = p_shm_get_size(shm);

			if (err != NULL)
			{
				wimp_log_fail("Failed to find data: %s %s %d\n", name, p_error_get_message(err), p_error_get_code(err));
				p_shm_free(shm);
				return NULL;
			}

			//Update the data pointer of the arena then return
			data_table[i].data_arena._data = p_shm_get_address(shm);

			//Set the local instance first
			s_Arena._arena = &data_table[i].data_arena;
			s_Arena._shm = shm;

			return &s_Arena;
		}
	}
	return NULL;
}

void wimp_data_stop_access(const char* name, WimpDataArena* arena)
{
	//TODO - there will be locking
	s_Arena._arena = NULL;
	p_shm_free(s_Arena._shm);
	s_Arena._shm = NULL;
	*arena = NULL;
	return;
}