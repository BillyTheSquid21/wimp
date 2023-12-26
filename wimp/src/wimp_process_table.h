#ifndef WIMP_PROCESS_TABLE
#define WIMP_PROCESS_TABLE

#include <stdint.h>
#include <plibsys.h>
#include <ptypes.h>
#include <plist.h>
#include <phashtable.h>
#include <utility/HashString.h>
#include <wimp_process.h>

#define WIMP_PROCESS_TABLE_SUCCESS 0
#define WIMP_PROCESS_TABLE_FAIL -1
#define WIMP_PROCESS_TABLE_MAX_LENGTH 128 //If need to track more processes, rethink

//TODO make opaque
typedef struct _WimpProcessData
{
	const char* process_domain;
	int32_t process_port;
	PSocket* process_connection;
} *WimpProcessData;

typedef struct _WimpProcessTable
{
	HashString* hash_table;
	size_t table_length;
} WimpProcessTable;

WimpProcessTable wimp_create_process_table();

int32_t wimp_process_table_add(WimpProcessTable* table, const char* process_name, const char* process_domain, int32_t process_port, PSocket* connection);

int32_t wimp_process_table_remove(WimpProcessTable* table, const char* process_name);

int32_t wimp_process_table_get(WimpProcessData* data, WimpProcessTable table, const char* process_name);

size_t wimp_process_table_length(WimpProcessTable table);

void wimp_process_table_free(WimpProcessTable table);

#endif