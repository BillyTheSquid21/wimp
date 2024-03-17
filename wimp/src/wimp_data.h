#ifndef WIMP_DATA_H
#define WIMP_DATA_H

#include <stdint.h>
#include <plibsys.h>
#include <utility/HashString.h>

/*
* The idea of the file/data system is to provide a way for using data
* between processes without duplicating, and to handle whether data
* needs to go to a local process or over the network with the same
* interface.
* 
* This means files will not be directly accessed, and a handle will
* be passed around to point to where the data needs to come from.
* 
* Data table and the generated handles are 2-Way linked. Every
* server has a data table. Arena is used for the data to make
* transfer and allocation vastly easier for complex structures.
* 
* The process of how this system works:
* 1. Either a file is loaded, or a space is reserved for data to go to.
*    (it is up to the end process to know how to read the data) - this
*    is done with an arena allocator
* 2. A process asks the table for a handle to the data - this connects
*    the handle to the table, and so allows controlling for synchronous
*    access - if done over the network this will download the data to
*    the table behind the scenes - access still looks the same
*    TODO - figure out how to avoid duplicates over the network method
* 3. The process accesses the data - it can do this in one of two ways:
*    a. Calling "wimp_data_start_read" - multiple processes can read at
*       the same time, and this provides a const ptr that will stay
*       safe to access until the corresponding end is called
*    b. Calling "wimp_data_start_read_write" - only one process at once
*       can access in this mode, and this provides a non const pointer
*       and also allows requesting the table for more memory. If this
*       happens, all the handles will have the size updated
* 4. After access, the corresponding end is called. The start and end
*    lock the data in their corresponding way
* 5. If not using the data anymore, tell the table to destroy the
*    header - once all headers are destroyed, can unload the data
* 6. If the table is freed, pass ownership of the
*    table to the parent if the table has any active handles
*/

/*
* The struct that has ownership of the child data
* Ownership can be transferred, once the new table copies the data it
* updates the ptr in the handle.
*/
struct _WimpDataTable;

/*
* The struct that you use to access shared data
* Is generated and destroyed by the source of the data and is tracked
*/
struct _WimpDataHandle;

/*
* The struct that contains data
*/
struct _WimpData;

typedef struct _WimpDataHandle
{
	char* name;
	size_t size;
	struct _WimpDataTable* _table;
} *WimpDataHandle;

typedef struct _WimpDataTable
{
	HashString* _hash_table;
	size_t _table_length;
} *WimpDataTable;

typedef struct _WimpData
{
	char* name;
	size_t size;
	PMutex* read_lock;
	PMutex* write_lock;
	WimpDataHandle* handles;
	//ARENA data_arena; //TODO - implement
} WimpData;

//Data table functions

int32_t wimp_create_data_table(WimpDataTable* table);

void wimp_data_table_free(WimpDataTable table);

int32_t wimp_data_table_append(WimpDataTable table, WimpDataTable table_appended);

int32_t wimp_data_table_load_file(WimpDataTable table, const char* file_path, const char* data_name);

int32_t wimp_data_table_allocate_data(WimpDataTable table, size_t max_size, const char* data_name);

int32_t wimp_data_table_unload_data(WimpDataTable table, const char* data_name);

WimpDataHandle wimp_data_table_get_data(WimpDataTable table, const char* data_name);

//Handle functions

void wimp_data_handle_free(WimpDataHandle* handle);

//const ARENA* wimp_data_start_read(WimpDataHandle handle);

//ARENA* wimp_data_start_read_write(WimpDataHandle handle);

void wimp_data_end_read(WimpDataHandle handle);

void wimp_data_end_read_write(WimpDataHandle handle);

#endif