#ifndef WIMP_PROCESS_TABLE
#define WIMP_PROCESS_TABLE

#include <stdint.h>
#include <plibsys.h>
#include <utility/HashString.h>
#include <wimp_process.h>

#define WIMP_PROCESS_TABLE_SUCCESS 0
#define WIMP_PROCESS_TABLE_FAIL -1
#define WIMP_PROCESS_TABLE_MAX_LENGTH 128 //If need to track more processes, rethink
#define WIMP_PROCESS_ACTIVE 1
#define WIMP_PROCESS_INACTIVE 0

/*
* The data stored for a process
*/
typedef struct _WimpProcessData
{
	char* process_domain;
	PSocket* process_connection;
	int32_t process_port;
	int32_t process_active;
} *WimpProcessData;

/*
* The process table struct. Should not access the members directly.
*/
typedef struct _WimpProcessTable
{
	HashString* _hash_table;
	size_t _table_length;
} WimpProcessTable;

/*
* Creates a new process table.
*/
WimpProcessTable wimp_create_process_table(void);

/*
* Adds a new process to the table.
* 
* @param table The pointer to the process table to add to
* @param process_name The name of the process to add
* @param process_domain The domain that the process runs on
* @param process_port The port that the process runs on
* @param connection The connection to the server for sending instructions to
* 
* @return Returns either WIMP_PROCESS_TABLE_SUCCESS or WIMP_PROCESS_TABLE_FAIL
*/
int32_t wimp_process_table_add(WimpProcessTable* table, const char* process_name, const char* process_domain, int32_t process_port, PSocket* connection);

/*
* Removes a process from the table
* 
* @param table The pointer to the process table to remove from
* @param process_name The name of the process to remove
* 
* @return Returns either WIMP_PROCESS_TABLE_SUCCESS or WIMP_PROCESS_TABLE_FAIL
*/
int32_t wimp_process_table_remove(WimpProcessTable* table, const char* process_name);

/*
* Gets the data for a process in the table
* 
* @param data The pointer to the location to store the returned data in.
* @param table The pointer to the process table to get from
* @param process_name The name of the process to get
* 
* @return Returns either WIMP_PROCESS_TABLE_SUCCESS or WIMP_PROCESS_TABLE_FAIL
*/
int32_t wimp_process_table_get(WimpProcessData* data, WimpProcessTable table, const char* process_name);

/*
* Removes the data for a process from the table
* 
* @param table The pointer to the process table to remove from
* @param process_name The name of the process to remove
* 
* 
*/
int32_t wimp_process_table_remove(WimpProcessTable* table, const char* process_name);

/*
* Gets the length of the table
* 
* @param table The table to get the length of
* 
* @return Returns the length of the table
*/
size_t wimp_process_table_length(WimpProcessTable table);

/*
* Frees the memory allocated by the table
* 
* @param table The table to free
*/
void wimp_process_table_free(WimpProcessTable table);

#endif