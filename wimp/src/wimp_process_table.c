#include <wimp_process_table.h>
#include <stdlib.h>

void wimp_process_data_free(void* data)
{
	WimpProcessData proc_data = (WimpProcessData)data;
	free(proc_data->process_domain);
	free(proc_data);
	return;
}

WimpProcessTable wimp_create_process_table()
{
	WimpProcessTable t;
	t._hash_table = HashString_create(WIMP_PROCESS_TABLE_MAX_LENGTH);
	t._table_length = 0;
	return t;
}

int32_t wimp_process_table_add(WimpProcessTable* table, const char* process_name, const char* process_domain, int32_t process_port, PSocket* connection)
{
	if (table == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	//Check if a process with that name already exists before continuing
	if (HashString_find(table->_hash_table, process_name) != NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	//Create the process data which will be freed with rest of table
	WimpProcessData process_data = malloc(sizeof(struct _WimpProcessData));
	if (process_data == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}
	process_data->process_domain = NULL;

	size_t dom_bytes = (strlen(process_domain) + 1) * sizeof(char);
	char* dom_str = malloc(dom_bytes);
	if (dom_str == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}
	memcpy(dom_str, process_domain, dom_bytes);
	process_data->process_domain = dom_str;
	process_data->process_port = process_port;
	process_data->process_connection = connection;
	process_data->process_active = WIMP_PROCESS_INACTIVE;

	if (HashString_add(table->_hash_table, process_name, process_data) != 0)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}
	table->_table_length++;

	return WIMP_PROCESS_TABLE_SUCCESS;
}

int32_t wimp_process_table_remove(WimpProcessTable* table, const char* process_name)
{
	if (table == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	HashStringEntry* entry = HashString_find(table->_hash_table, process_name);
	if (entry == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	wimp_process_data_free(entry->value);
	
	if (HashString_remove(table->_hash_table, process_name) != 0)
	{
		table->_table_length--;
		return WIMP_PROCESS_TABLE_FAIL;
	}

	table->_table_length--;
	return WIMP_PROCESS_TABLE_SUCCESS;
}

int32_t wimp_process_table_get(WimpProcessData* data, WimpProcessTable table, const char* process_name)
{
	if (process_name == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	HashStringEntry* val = HashString_find(table._hash_table, process_name);
	if (val == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	*data = (WimpProcessData)(val->value);
	return WIMP_PROCESS_TABLE_SUCCESS;
}

size_t wimp_process_table_length(WimpProcessTable table)
{
	return table._table_length;
}

void wimp_process_table_free(WimpProcessTable table)
{
	//Iterate all the data and free, then free the table
	if (table._table_length > 0)
	{
		HashStringEntry* entry = NULL;
		int index = -1;

		HashString_firstEntry(table._hash_table, &entry, &index);

		while (entry != NULL)
		{
			wimp_process_data_free(entry->value);
			entry = entry->next;
		}
	}

	HashString_destroy(table._hash_table);
}