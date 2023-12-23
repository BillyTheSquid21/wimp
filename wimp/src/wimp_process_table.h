#ifndef WIMP_PROCESS_TABLE
#define WIMP_PROCESS_TABLE

#include <stdint.h>
#include <plibsys.h>
#include <ptypes.h>
#include <plist.h>
#include <phashtable.h>
#include <wimp_process.h>

#define WIMP_PROCESS_TABLE_SUCCESS 0
#define WIMP_PROCESS_TABLE_FAIL -1

//TODO make opaque
typedef struct _WimpProcessData
{
	const char* process_domain;
	int32_t process_port;
	PSocket* process_connection;
} *WimpProcessData;

typedef struct _WimpProcessTable
{
	PHashTable* table;
} WimpProcessTable;

WimpProcessTable wimp_create_process_table();

int32_t wimp_process_table_add(WimpProcessTable table, const char* process_name, const char* process_domain, int32_t process_port, PSocket* connection);

int32_t wimp_process_table_get(WimpProcessData* data, WimpProcessTable table, const char* process_name);

size_t wimp_process_table_length(WimpProcessTable table);

void wimp_process_table_free(WimpProcessTable table);

#endif