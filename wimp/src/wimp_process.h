///
/// @file
///
/// This header defines the interfaces to the wimp_process
///

#ifndef WIMP_PROCESS_H
#define WIMP_PROCESS_H

#include <wimp_core.h>
#include <wimp_reciever.h>

/// @brief The result of a WIMP process operation
enum WimpProcessResult
{
	WIMP_PROCESS_SUCCESS = 0, ///< Result if process operation is successful
	WIMP_PROCESS_FAIL = -1,   ///< Result if process operation fails for an unspecified reason
};

#define MAX_PORT_STRING_LEN 6
#define MAX_DIRECTORY_PATH_LEN 1024

///
/// Processes can be launched, and the recieve threads and the end processes are
/// kept track of with a lookup table.
///

/// @brief Main function type pointer
typedef int (*MAIN_FUNC_PTR)(int, const char*);

/// @brief String representation of a port
typedef char WimpPortStr[MAX_PORT_STRING_LEN];

///
/// @brief The main arguments for a main function
///
/// Is a wrapper for the normal entry args so can be called from a thread.
///
typedef struct _WimpMainEntry
{
	char** argv;
	int argc;
} *WimpMainEntry;

///
/// @brief Starts the wimp library
///
/// Should be called at the start of a process
///
/// @return Returns either WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
///
WIMP_API int32_t wimp_init(void);

///
/// @brief Shuts down the wimp library
///
WIMP_API void wimp_shutdown(void);

///
/// @brief Starts a library process
///
/// A library process is one called from a statically or dynamically linked library
/// 
/// @param process_name The name of the process to create
/// @param main_func The pointer to the main function, which should be set up for being a library process not an executable one
/// @param priority The puthread thread priority
/// @param entry The entry arguments
/// 
/// @return Returns either WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
///
WIMP_API int32_t wimp_start_library_process(const char* process_name, MAIN_FUNC_PTR main_func, enum PUThreadPriority_ priority, WimpMainEntry entry);

///
/// @brief Starts an executable process
///
/// Currently must be located downstream of the location of the calling executable 
/// to avoid arbitrary execution. If needed, may create a specific function for launching
/// arbitrary exe's, although this might be very unsafe.
/// 
/// For now cannot automatically set exe priority. The user could supply
/// this from command line args and set normally.
/// 
/// @param process_name The name of the process to create
/// @param executable The executable relative to the location the program binary is running from (e.g. bin directory)
/// @param entry The entry arguments
/// 
/// @return Returns either WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
///
WIMP_API int32_t wimp_start_executable_process(const char* process_name, const char* executable, WimpMainEntry entry);

#ifdef _WIN32
typedef struct _PROG_ENTRY* PROG_ENTRY;

///
/// Helper function for launching binaries in windows (to be launched as thread)
///
/// @param entry The entry struct containing the path and args
///
static void wimp_launch_binary(PROG_ENTRY entry);
#endif

///
/// @brief Gets the directory of the currently running executable
/// 
/// @param path Pointer to a buffer to write the path into (must be at least MAX_DIRECTORY_PATH_LEN)
/// 
/// @return Returns WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
///
WIMP_API int32_t wimp_get_running_executable_directory(char* path);

///
/// @brief Creates the arguments for a main function
///
/// Creates a persistent collection of arguments to supply, which can be freed after the library main thread is finished
/// 
/// @param argc The number of arguments supplied
/// @param ... The arguments, which must be null terminated strings
/// 
/// @return Returns the entry arguments
///
WIMP_API WimpMainEntry wimp_get_entry(int32_t argc, ...);

///
/// @brief Frees the entry arguments
///
/// Must be called in the thread using the entry arguments (i.e. the child)
/// 
/// @param entry The entry arguments to free
///
WIMP_API void wimp_free_entry(WimpMainEntry entry);

///
/// @brief Gives the number of an unused port
///
/// Gets an unused local port from the OS by binding a dummy socket
/// 
/// @return Returns the port number if successful, otherwise returns WIMP_PROCESS_FAIL
///
WIMP_API int32_t wimp_assign_unused_local_port(void);

///
/// @brief Converts the numerical port to a stack string representation
/// 
/// @param port The numerical representation of the port
/// @param string_out The location to write the string to
/// 
/// @return Returns either WIMP_PROCESS_SUCCESS or WIMP_PROCESS_FAIL
///
WIMP_API int32_t wimp_port_to_string(int32_t port, WimpPortStr string_out);

#endif