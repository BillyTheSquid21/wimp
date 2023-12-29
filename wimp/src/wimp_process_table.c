#include <wimp_process_table.h>

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

int32_t wimp_process_accept(WimpProcessTable* table, PSocket* server, uint8_t* recbuffer, uint8_t* sendbuffer)
{
	printf("WAITING FOR CONNECTION...\n");
	// Blocks until connection accept happens by default -- this can be changed
	PSocket* con = p_socket_accept(server, NULL);

	if (con != NULL)
	{
		//Only blocking call, recieve handshake from reciever
		int handshake_size = p_socket_receive(con, recbuffer, WIMP_MESSAGE_BUFFER_BYTES, NULL);
			
		if (handshake_size <= 0)
		{
			printf("HANDSHAKE FAILED!\n");
			return 1;
		}

		//Check start of handshake
		WimpHandshakeHeader potential_handshake = *((WimpHandshakeHeader*)((void*)recbuffer));
		size_t offset = sizeof(WimpHandshakeHeader);
		if (potential_handshake.handshake_header != WIMP_RECIEVER_HANDSHAKE)
		{
			printf("HANDSHAKE FAILED!\n");
			return 1;
		}
		printf("test_proces HANDSHAKE SUCCESS!\n");

		//Get process name
		char* proc_name = &recbuffer[offset];

		//Add connection to the process table
		WimpProcessData procdat = NULL;
		printf("Adding to test_process process table: %s\n", proc_name);
		if (wimp_process_table_get(&procdat, *table, proc_name) == WIMP_PROCESS_TABLE_FAIL)
		{
			printf("Process not found! %s\n", proc_name);
			return -1;
		}
		printf("Process added!\n");

		procdat->process_connection = con;

		//Clear rec buffer
		WIMP_ZERO_BUFFER(recbuffer);

		//Send handshake back with no process name this time
		WimpHandshakeHeader* sendheader = ((WimpHandshakeHeader*)sendbuffer);
		sendheader->handshake_header = WIMP_RECIEVER_HANDSHAKE;
		sendheader->process_name_bytes = 0;

		p_socket_send(con, sendbuffer, sizeof(WimpHandshakeHeader), NULL);
		return 0;
	}
	else
	{
		printf("Can't make con, tried and failed...\n");
		return -1;
	}
	return -1;
}