#ifdef __unix__
#define _GNU_SOURCE
#endif

#include <wimp_process.h>
#include <wimp_log.h>
#include <time.h>
#include <stdlib.h>
#include <utility/sds.h>
#include <patomic.h>

static pint s_init_ref_counter = 0;

#ifdef _WIN32

#include <windows.h>
#include <sys/types.h>
#include <io.h>

#define F_OK 0
#define access _access

#define WIMP_EXE_WINDOW_SHOW SW_HIDE

typedef struct _PROG_ENTRY
{
	sds path;
	sds args;
}* PROG_ENTRY;

void wimp_launch_binary(PROG_ENTRY entry)
{
	wimp_log("Launching: %s With args: %s\n", entry->path, entry->args);
	ShellExecute(NULL, "open", entry->path, entry->args, NULL, WIMP_EXE_WINDOW_SHOW);
	
	wimp_log("Closing: %s\n", entry->path);
	sdsfree(entry->path);
	sdsfree(entry->args);
	free(entry);
}

int32_t wimp_start_executable_process(const char* process_name, const char* executable, WimpMainEntry entry)
{
	//Get the directory of the running process
	//Use malloc to preserve outside function stack frame (is freed above)
	char path_buffer[MAX_DIRECTORY_PATH_LEN];
	memset(&path_buffer[0], 0, MAX_DIRECTORY_PATH_LEN);

	//Get the path of the currently running executable
	GetModuleFileName(NULL, path_buffer, MAX_DIRECTORY_PATH_LEN);

	//Erase the file part from the string end
	size_t current_dir_bytes = strlen(path_buffer) * sizeof(char);
	size_t last_slash_index = MAX_DIRECTORY_PATH_LEN;
	for (size_t i = current_dir_bytes; i > 0; --i)
	{
		if (path_buffer[i] == '/' || path_buffer[i] == '\\')
		{
			last_slash_index = i;
			break;
		}
	}

	if (last_slash_index == MAX_DIRECTORY_PATH_LEN)
	{
		wimp_log_fail("Issue reading the path of the program! %s\n", path_buffer);
		return WIMP_PROCESS_FAIL;
	}

	//Blank everything after the index (except slash)
	memset(&path_buffer[last_slash_index + 1], 0, MAX_DIRECTORY_PATH_LEN - last_slash_index - 1);

	//Create the heap string for appending
	sds path = sdsnew(&path_buffer[0]);

	//Add the rest of the path specified - TODO allow ../../ format - currently can't!
	path = sdscat(path, executable);

	//If on windows, add ".exe"
	path = sdscat(path, ".exe");

	//Check the file exists
	if (access(path, F_OK) != 0)
	{
		wimp_log_fail("%s was not found!\n", path);
		return WIMP_PROCESS_FAIL;
	}

	//Make the entry args
	PROG_ENTRY prog_entry = malloc(sizeof(struct _PROG_ENTRY));
	if (prog_entry == NULL)
	{
		return WIMP_PROCESS_FAIL;
	}

	prog_entry->path = path;
	prog_entry->args = sdsempty();

	//For windows collate the other args
	//The shell launch function wants it in this format
	for (int i = 0; i < entry->argc; ++i)
	{
		prog_entry->args = sdscat(prog_entry->args, entry->argv[i]);
		if (i < entry->argc - 1)
		{
			prog_entry->args = sdscat(prog_entry->args, " ");
		}
	}

	//Launch the windows version of the function
	PUThread* process_thread = p_uthread_create((PUThreadFunc)&wimp_launch_binary, prog_entry, false, process_name);
	if (process_thread == NULL)
	{
		wimp_log_fail("Failed to create thread: %s", process_name);
		return WIMP_PROCESS_FAIL;
	}
	return WIMP_PROCESS_SUCCESS;
}
#endif

#ifdef __unix__

#include <unistd.h>
#include <stdio.h>

int32_t wimp_start_executable_process(const char* process_name, const char* executable, WimpMainEntry entry)
{
	//Get the directory of the running process
	//Use malloc to preserve outside function stack frame (is freed above)
	char path_buffer[MAX_DIRECTORY_PATH_LEN];
	memset(&path_buffer[0], 0, MAX_DIRECTORY_PATH_LEN);

	//Get the path of the currently running executable
	ssize_t linklen = readlink("/proc/self/exe", path_buffer, MAX_DIRECTORY_PATH_LEN);
	path_buffer[linklen] = '\0';

	//Erase the file part from the string end
	size_t current_dir_bytes = strlen(path_buffer) * sizeof(char);
	size_t last_slash_index = MAX_DIRECTORY_PATH_LEN;
	for (size_t i = current_dir_bytes; i > 0; --i)
	{
		if (path_buffer[i] == '/' || path_buffer[i] == '\\')
		{
			last_slash_index = i;
			break;
		}
	}

	if (last_slash_index == MAX_DIRECTORY_PATH_LEN)
	{
		wimp_log_fail("Issue reading the path of the program! %s\n", path_buffer);
		return WIMP_PROCESS_FAIL;
	}

	//Blank everything after the index (except slash)
	memset(&path_buffer[last_slash_index + 1], 0, MAX_DIRECTORY_PATH_LEN - last_slash_index - 1);

	//Create the heap string for appending
	sds path = sdsnew(&path_buffer[0]);

	//Add the rest of the path specified - TODO allow ../../ format - currently can't!
	path = sdscat(path, executable);

	//Check the file exists
	if (access(path, F_OK) != 0)
	{
		wimp_log_fail("%s was not found!\n", path);
		return WIMP_PROCESS_FAIL;
	}

	//For linux put all the arguments in one space separated string
	//First add space after executable
	path = sdscat(path, " ");

	for (int i = 0; i < entry->argc; ++i)
	{
		path = sdscat(path, entry->argv[i]);
		if (i < entry->argc - 1)
		{
			path = sdscat(path, " ");
		}
	}

	//Launch the linux version of the function
	wimp_log("Launching: %s\n", path);
	FILE* f = popen(path, "r");
	if (f == NULL)
	{
		wimp_log_fail("Error starting executable! %p\n", f);
		return -1;
	}
	sdsfree(path);
	return WIMP_PROCESS_SUCCESS;
}

#endif

int32_t wimp_start_library_process(const char* process_name, MAIN_FUNC_PTR main_func, enum PUThreadPriority_ priority, WimpMainEntry entry)
{
	wimp_log_important("Starting %s!\n", process_name);
	PUThread* process_thread = p_uthread_create_full((PUThreadFunc)main_func, entry, false, priority, 0, process_name);
	if (process_thread == NULL)
	{
		wimp_log_fail("Failed to create thread: %s", process_name);
		return WIMP_PROCESS_FAIL;
	}
	return WIMP_PROCESS_SUCCESS;
}

int32_t wimp_init(void)
{
	if (p_atomic_int_get(&s_init_ref_counter) == 0)
	{
		wimp_log_important("WIMP Init\n");
		p_libsys_init();
	}
	p_atomic_int_inc(&s_init_ref_counter);
	return WIMP_PROCESS_SUCCESS;
}

void wimp_shutdown(void)
{
	p_atomic_int_dec_and_test(&s_init_ref_counter);
	if (p_atomic_int_get(&s_init_ref_counter) == 0)
	{
		wimp_log_important("WIMP Shutdown\n");
		p_libsys_shutdown();
	}
	assert(p_atomic_int_get(&s_init_ref_counter) >= 0 && "More processes called shutdown than init!\n");
}

WimpMainEntry wimp_get_entry(int32_t argc, ...)
{
	va_list argp;
	va_start(argp, argc);
	WimpMainEntry main_entry = malloc(sizeof(struct _WimpMainEntry));
	if (main_entry == NULL)
	{
		va_end(argp);
		return NULL;
	}

	//Process the command line args supplied (add null ptr at the end)
	size_t argv_bytes = ((size_t)argc * sizeof(char*)) + sizeof(void*);
	main_entry->argc = argc;
	main_entry->argv = malloc(argv_bytes);
	if (main_entry->argv == NULL)
	{
		free(main_entry);
		va_end(argp);
		return NULL;
	}

	for (size_t i = 0; i < argc; ++i)
	{
		const char* arg = va_arg(argp, const char*);
		if (arg == NULL)
		{
			for(int32_t j = (int32_t)i - 1; j >= 0; j--)
			{
				free(main_entry->argv[j]); //Free all the previous malloc args
			}
			free(main_entry->argv);
			free(main_entry);
			va_end(argp);
			return NULL;
		}
		
		size_t arg_bytes = (strlen(arg) + 1) * sizeof(char); //Include the null terminator
		char* argv = malloc(arg_bytes);
		if (argv == NULL)
		{
			for(int32_t j = (int32_t)i - 1; j >= 0; j--)
			{
				free(main_entry->argv[j]); //Free all the previous malloc args
			}
			free(main_entry->argv);
			free(main_entry);
			va_end(argp);
			return NULL;
		}
		main_entry->argv[i] = argv;
		memcpy(main_entry->argv[i], arg, arg_bytes);
	}
	va_end(argp);

	//Zero last element of argv pointers to play nice with linux
	main_entry->argv[argc] = NULL;

	return main_entry;
}

void wimp_free_entry(WimpMainEntry entry)
{
	//Free the args first
	for (int i = 0; i < entry->argc; ++i)
	{
		free(entry->argv[i]);
	}
	free(entry);
	return;
}

int32_t wimp_assign_unused_local_port(void)
{
	//bind dummy socket, get the port and return
	PSocket* reciever_socket;
    PSocketAddress* reciever_address;

    //Construct address for client, which should be listening
    reciever_address = p_socket_address_new("127.0.0.1", 0);
    if (reciever_address == NULL)
    {
		wimp_log_fail("Failed to bind to unused port!\n");
		p_socket_address_free(reciever_address);
        return WIMP_PROCESS_FAIL;
    }

	if ((reciever_socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		wimp_log_fail("Failed to bind to unused port!\n");
		p_socket_address_free(reciever_address);
		return WIMP_PROCESS_FAIL;
	}

	if (!p_socket_bind(reciever_socket, reciever_address, TRUE, NULL))
	{
		wimp_log_fail("Failed to bind to unused port!\n");
		p_socket_address_free(reciever_address);
		p_socket_free(reciever_socket);
		return WIMP_PROCESS_FAIL;
	}

	PSocketAddress* bound_address = p_socket_get_local_address(reciever_socket, NULL);
	
	int32_t port = p_socket_address_get_port(bound_address);
	p_socket_address_free(reciever_address);
	p_socket_address_free(bound_address);
    p_socket_close(reciever_socket, NULL);
	return port;
}

int32_t wimp_port_to_string(int32_t port, WimpPortStr string_out)
{
	memset(string_out, 0, MAX_PORT_STRING_LEN);
	sprintf(string_out, "%d", port);
	return WIMP_PROCESS_SUCCESS;
}