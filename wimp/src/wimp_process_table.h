///
/// @file
///
/// This header defines the interfaces to the wimp_process_table
///

#ifndef WIMP_PROCESS_TABLE
#define WIMP_PROCESS_TABLE

#include <stdint.h>
#include <plibsys.h>
#include <utility/HashString.h>
#include <utility/sds.h>
#include <wimp_core.h>
#include <wimp_process.h>

#define WIMP_PROCESS_TABLE_MAX_LENGTH 128 //If need to track more processes, rethink
#define WIMP_PROCESS_ACTIVE 1
#define WIMP_PROCESS_INACTIVE 0

/// @brief The result of WIMP process table operations
enum WimpProcessTableResult
{
	WIMP_PROCESS_TABLE_SUCCESS = 0,	///< Result if process table operation is successful
	WIMP_PROCESS_TABLE_FAIL    = -1,///< Result if process table operation fails for an unspecified reason
};

///
/// @brief The relation a process has in the table
/// 
/// This allows tracking process relationships, and is relative to the process table owner
///
typedef enum _WimpRelation
{
	WIMP_Process_Unknown		= 0x00, ///< An unknown process, which is usually an error
	WIMP_Process_Child			= 0x01, ///< A child process which belongs to this process
	WIMP_Process_Parent			= 0x02, ///< A parent process, which owns this process
	WIMP_Process_Independent	= 0x03, ///< A process that has no defined relationship with this process
} WimpRelation;

///
/// @brief The data stored for a process
///
typedef struct _WimpProcessData
{
	sds process_domain;			///< String representation of the process domain
	PSocket* process_connection;///< Connection socket to the process
	int32_t process_port;		///< Port the process runs on
	int16_t process_active;		///< Whether the process is active or not
	int16_t process_relation;	///< Relationship of the process to this process
} *WimpProcessData;

///
/// Defines the process table
///
typedef struct _WimpProcessTable
{
	HashString* _hash_table;
	size_t _table_length;
} WimpProcessTable;

///
/// @brief Creates a new process table.
///
/// @return Returns a new process table
///
WIMP_API WimpProcessTable wimp_create_process_table(void);

///
/// @brief Adds a new process to the table.
/// 
/// @param table The pointer to the process table to add to
/// @param process_name The name of the process to add
/// @param process_domain The domain that the process runs on
/// @param process_port The port that the process runs on
/// @param relation The relationship of the process to this process
/// @param connection The connection to the server for sending instructions to
/// 
/// @return Returns either WIMP_PROCESS_TABLE_SUCCESS or WIMP_PROCESS_TABLE_FAIL
///
WIMP_API int32_t wimp_process_table_add(WimpProcessTable* table, const char* process_name, const char* process_domain, int32_t process_port, enum _WimpRelation relation, PSocket* connection);

///
/// @brief Removes a process from the table
/// 
/// @param table The pointer to the process table to remove from
/// @param process_name The name of the process to remove
/// 
/// @return Returns either WIMP_PROCESS_TABLE_SUCCESS or WIMP_PROCESS_TABLE_FAIL
///
WIMP_API int32_t wimp_process_table_remove(WimpProcessTable* table, const char* process_name);

///
/// @brief Gets the data for a process in the table
/// 
/// @param data The pointer to the location to store the returned data in.
/// @param table The pointer to the process table to get from
/// @param process_name The name of the process to get
/// 
/// @return Returns either WIMP_PROCESS_TABLE_SUCCESS or WIMP_PROCESS_TABLE_FAIL
///
WIMP_API int32_t wimp_process_table_get(WimpProcessData* data, WimpProcessTable table, const char* process_name);

///
/// @brief Removes the data for a process from the table
/// 
/// @param table The pointer to the process table to remove from
/// @param process_name The name of the process to remove
///
///
WIMP_API int32_t wimp_process_table_remove(WimpProcessTable* table, const char* process_name);

///
/// @brief Gets the length of the table
/// 
/// @param table The table to get the length of
/// 
/// @return Returns the length of the table
///
WIMP_API size_t wimp_process_table_length(WimpProcessTable table);

///
/// @brief Frees the memory allocated by the table
/// 
/// @param table The table to free
///
WIMP_API void wimp_process_table_free(WimpProcessTable table);

#endif