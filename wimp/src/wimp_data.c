#include <wimp_data.h>
#include <utility/thread_local.h>

static thread_local PShm* s_DataTable = NULL;
static thread_local HashString* s_DataTableCacheMap; //Lookup for the cache
static thread_local PShm* s_DataTableCache[WIMP_MAX_SHARED_SLOTS]; //Local Cache for the PShm locations for linking
static thread_local char* s_MemoryName = NULL;
static thread_local bool s_IsTableLinked = false;

#define TABLE_LENGTH sizeof(WimpDataSlot) * WIMP_MAX_SHARED_SLOTS

static PShm* wimp_create_fresh_shm(const char* memory_name, size_t length, PError** error)
{
	//Create a shared memory segment for the table
	//To ensure nothing survived an application crash, free then create again
	PError* err = NULL;
	PShm* shm = p_shm_new(
		memory_name, 
		length,
		P_SHM_ACCESS_READWRITE, 
		error
	);

	p_shm_take_ownership(shm);
	p_shm_free(shm);

	shm = p_shm_new(
		memory_name, 
		length,
		P_SHM_ACCESS_READWRITE, 
		error
	);
	return shm;
}

int32_t wimp_data_init(const char* memory_name)
{
	PError* err = NULL;
	PShm* shm = wimp_create_fresh_shm(memory_name, TABLE_LENGTH, &err);

	//Continue with the fresh memory
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
		p_atomic_int_set(&table_data[i].share_counter, 0);
		memset(&table_data[i].name[0], 0, WIMP_SHARED_SLOT_MAX_NAME_BYTES);
	}

	s_DataTable = shm;
	s_IsTableLinked = true;
	s_DataTableCacheMap = HashString_create(WIMP_MAX_SHARED_SLOTS);
	wimp_log_success("Created shared data table for program! %p\n", table_data);
	return WIMP_DATA_SUCCESS;
}

int32_t wimp_data_link_to_process(const char* memory_name)
{
	//Check if already linked
	if (s_IsTableLinked)
	{
		wimp_log("Data table already linked!\n");
		return WIMP_DATA_FAIL;
	}

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

	//Zero the PShm cache
	for (int i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
	{
		s_DataTableCache[i] = NULL;
	}

	//Init the cache lookup
	s_DataTableCacheMap = HashString_create(WIMP_MAX_SHARED_SLOTS);

	s_DataTable = shm;
	s_MemoryName = memory_name;
	s_IsTableLinked = true;
	wimp_log_success("Successfully linked to the data table\n");
	return WIMP_DATA_SUCCESS;
}

void wimp_data_unlink_from_process(void)
{
	if (s_IsTableLinked)
	{
		p_shm_free(s_DataTable);

		//Clear the table cache, unlinking all of it
		for (int i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
		{
			if (s_DataTableCache[i] != NULL)
			{
				p_shm_free(s_DataTableCache[i]);
				s_DataTableCache[i] = NULL;
			}
		}

		//Destroy the cache map
		HashString_destroy(s_DataTableCacheMap);
		s_IsTableLinked = false;
		s_DataTable = NULL;
	}
	return;
}

void wimp_data_free(void)
{
	if (s_IsTableLinked)
	{
		WimpDataSlot* data_table = p_shm_get_address(s_DataTable);

		//Link all SHM of the data if exists
		//Then take ownership and free
		//This ensures that any residual data is cleared properly by master
		for (int i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
		{
			if (data_table[i].share_counter >= 0)
			{
				//If not already cached, link
				if (s_DataTableCache[i] == NULL
					&& strcmp(data_table[i].name, "") != 0)
				{
					wimp_data_link_to_data(data_table[i].name);
				}

				//Take ownership of the shm and free
				p_shm_take_ownership(s_DataTableCache[i]);
				p_shm_free(s_DataTableCache[i]);
				s_DataTableCache[i] = NULL;
			}
		}

		HashString_destroy(s_DataTableCacheMap);

		//Clear the table
		for (int i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
		{
			data_table[i].share_counter = 0;
			data_table[i].size = 0;
			data_table[i].data_arena = sarena();
			memset(&data_table[i].name[0], 0, WIMP_SHARED_SLOT_MAX_NAME_BYTES);
		}

		s_IsTableLinked = false;
		s_DataTable = NULL;
	}
}

int32_t wimp_data_reserve(const char* reserved_name, size_t size)
{
	//Get the memory ptr
	WimpDataSlot* data_table = p_shm_get_address(s_DataTable);

	//Find a free slot
	p_shm_lock(s_DataTable, NULL);
	for (int i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
	{
		if (data_table[i].share_counter == 0)
		{
			//Copy the name
			memcpy(
				&data_table[i].name[0], 
				reserved_name, 
				(strlen(reserved_name) + 1) * sizeof(char)
			);

			PError* err = NULL;
			PShm* shm = wimp_create_fresh_shm(reserved_name, TABLE_LENGTH, &err);

			//Continue with the fresh memory
			psize shmsize = p_shm_get_size(shm);

			if (err != NULL || shmsize < size)
			{
				wimp_log_fail("Failed to allocate data: %s %d\n", p_error_get_message(err), p_error_get_code(err));
				p_shm_free(shm);
				p_shm_unlock(s_DataTable, NULL);
				return WIMP_DATA_FAIL;
			}

			//Init the arena
			sarena_init(&data_table[i].data_arena, NULL, size);

			//Cache the shm locally and link
			s_DataTableCache[i] = shm;
			HashString_add(s_DataTableCacheMap, reserved_name, (void*)i);

			data_table[i].share_counter++;
			data_table[i].size = size;
			p_shm_unlock(s_DataTable, NULL);
			return WIMP_DATA_SUCCESS;
		}
	}
	p_shm_unlock(s_DataTable, NULL);
	wimp_log_fail("Unable to find a free spot in data table!\n");
	return WIMP_DATA_FAIL;
}

int32_t wimp_data_link_to_data(const char* name)
{
	//Get the memory ptr
	WimpDataSlot* data_table = p_shm_get_address(s_DataTable);
	PError* err;

	//Check if the data is in the table
	p_shm_lock(s_DataTable, &err);
	for (int i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
	{
		if (data_table[i].share_counter != 0)
		{
			if (strcmp(data_table[i].name, name) == 0)
			{
				//Check if already linked
				if (s_DataTableCache[i] != NULL)
				{
					wimp_log("%s already linked!\n", name);
					p_shm_unlock(s_DataTable, NULL);
					return WIMP_DATA_FAIL;
				}

				//Get and cache the Shm
				PShm* shm = p_shm_new(
					name,
					data_table[i].size,
					P_SHM_ACCESS_READWRITE,
					&err
				);

				s_DataTableCache[i] = shm;
				data_table[i].share_counter++;
				p_shm_unlock(s_DataTable, NULL);

				//Cache the name and address in the map
				HashString_add(s_DataTableCacheMap, name, (void*)i);
				wimp_log_success("%s was successfully linked to\n", name);
				return WIMP_DATA_SUCCESS;
			}
		}
	}
	p_shm_unlock(s_DataTable, NULL);
	wimp_log_fail("%s not found for linking!\n", name);
	return WIMP_DATA_FAIL;
}

void wimp_data_unlink_from_data(const char* name)
{
	//Get the memory ptr
	WimpDataSlot* data_table = p_shm_get_address(s_DataTable);
	PError* err;

	//Check if the data is in the table
	p_shm_lock(s_DataTable, &err);
	for (int i = 0; i < WIMP_MAX_SHARED_SLOTS; ++i)
	{
		if (data_table[i].share_counter != 0)
		{
			if (strcmp(data_table[i].name, name) == 0)
			{
				//Decrease the counter and remove from cache
				data_table[i].share_counter--;
				p_shm_free(s_DataTableCache[i]);

				//Uncache the name and address
				HashString_remove(s_DataTableCacheMap, name);

				//If counter is 0, clear spot
				if (data_table[i].share_counter <= 0)
				{
					data_table[i].share_counter = 0;
					data_table[i].size = 0;
					sarena_init(&data_table[i].data_arena, NULL, 0);
					memset(&data_table[i].name[0], 0, WIMP_SHARED_SLOT_MAX_NAME_BYTES);
					wimp_log_important("%s was removed from shared data!\n", name);
				}
				p_shm_unlock(s_DataTable, NULL);
				return WIMP_DATA_SUCCESS;
			}
		}
	}
	p_shm_unlock(s_DataTable, NULL);
	return WIMP_DATA_FAIL;
}

int32_t wimp_data_access(WimpDataArena* arena, const char* name)
{
	//Find the data location in cache
	HashStringEntry* entry = HashString_find(s_DataTableCacheMap, name);
	if (entry == NULL)
	{
		wimp_log_fail("%s is not linked!\n", name);
		return WIMP_DATA_FAIL;
	}

	size_t data_index = (size_t)entry->value;
	PShm* shm = s_DataTableCache[data_index];
	uint8_t* data = p_shm_get_address(shm);

	/*
	* Lock the data here to prevent other access of the arena while
	* accessing.
	* 
	* Even though we access the data table, do not need to lock - are
	* accessing the arena, which is only written to on tow occasions:
	* 
	* 1. When being accessed here, so is protected by the shm lock
	* 2. When being removed due to a share counter of <=0 - this also
	*    cannot interfere with this access, as this accessing means that
	*    the share counter will remain >= 1
	*/
	p_shm_lock(shm, NULL);

	//Init the arena from the data table
	//Make a copy of the arena, is updated, then updates the data table
	//version.
	WimpDataSlot* data_table = p_shm_get_address(s_DataTable);
	arena->_arena = data_table[data_index].data_arena;
	arena->_arena._data = data;
	arena->_shm = shm;
	return WIMP_DATA_SUCCESS;
}

void wimp_data_stop_access(WimpDataArena* arena, const char* name)
{
	HashStringEntry* entry = HashString_find(s_DataTableCacheMap, name);
	if (entry == NULL)
	{
		wimp_log_fail("%s is not linked!\n", name);
		return;
	}

	size_t data_index = (size_t)entry->value;

	//Update the arena in the data table
	arena->_arena._data = NULL;
	WimpDataSlot* data_table = p_shm_get_address(s_DataTable);
	data_table[data_index].data_arena = arena->_arena;

	//Unlock the shm and invalidate the arena
	p_shm_unlock(arena->_shm, NULL);
	arena->_arena = sarena();
	return;
}