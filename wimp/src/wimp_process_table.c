#include <wimp_process_table.h>

WimpProcessTable wimp_create_process_table()
{
	WimpProcessTable t;
	t.table = p_hash_table_new();
	return t;
}

int32_t wimp_process_table_add(WimpProcessTable table, const char* process_name, const char* process_domain, int32_t process_port, PSocket* connection)
{
	//Check if a process with that name already exists before continuing
	if (p_hash_table_lookup(table.table, process_name) != -1)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	//Allocate for the name - will be freed with rest of table
	//The allocations are to give the table its own copy of data to ensure
	//persistence.
	size_t name_bytes = (strlen(process_name) + 1) * sizeof(char);
	char* name_str = malloc(name_bytes);
	if (name_str == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}
	memcpy(name_str, process_name, name_bytes);

	//Create the process data which will also be freed with rest of table
	WimpProcessData process_data = malloc(sizeof(struct _WimpProcessData));
	if (process_data == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	size_t dom_bytes = (strlen(process_domain) + 1) * sizeof(char);
	char* dom_str = malloc(dom_bytes);
	if (process_data->process_domain == NULL)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}
	memcpy(dom_str, process_domain, dom_bytes);
	process_data->process_domain = dom_str;

	process_data->process_port = process_port;
	process_data->process_connection = connection;

	p_hash_table_insert(table.table, name_str, process_data);

	return WIMP_PROCESS_TABLE_SUCCESS;
}

int32_t wimp_process_table_get(WimpProcessData* data, WimpProcessTable table, const char* process_name)
{
	ppointer val = p_hash_table_lookup(table.table, process_name);
	if (val == -1)
	{
		return WIMP_PROCESS_TABLE_FAIL;
	}

	*data = (WimpProcessData)(val);
	return WIMP_PROCESS_TABLE_SUCCESS;
}

size_t wimp_process_table_length(WimpProcessTable table)
{
	PList* list = p_hash_table_keys(table.table);
	size_t len = p_list_length(list);
	p_list_free(list);
	return len;
}

void wimp_process_data_free(ppointer data)
{
	WimpProcessData proc_data = (WimpProcessData)data;
	free(proc_data->process_domain);
	free(proc_data);
	return;
}

void wimp_process_table_free(WimpProcessTable table)
{
	//Get name list, then free all the process data and the key
	PList* keys_set = NULL;
	PList* current_key = NULL;
	keys_set = p_hash_table_keys(table.table);
	psize length = p_list_length(keys_set);
	current_key = keys_set;

	for (psize i = 0; i < length; ++i)
	{
		//Data is just a pointer to the heap string
		//Get this data, free the process at the lookup then free the string
		char* key_str = current_key->data;
		ppointer proc_data = p_hash_table_lookup(table.table, key_str);
		
		wimp_process_data_free(proc_data);
		free(key_str);

		if (current_key->next != NULL)
		{
			current_key = current_key->next;
		}
	}


	p_list_free(keys_set);
	p_hash_table_free(table.table);
}