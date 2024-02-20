#include <wimp_process.h>
#include <wimp_log.h>
#include <time.h>
#include <stdlib.h>

#define MAX_DIRECTORY_PATH_LEN 1024

#ifdef _WIN32

#include <windows.h>
#include <sys/types.h>
#include <io.h>

#define F_OK 0
#define access _access

#define WIMP_EXE_WINDOW_SHOW SW_HIDE

typedef struct _PROG_ENTRY
{
	char* path;
	char* args;
}* PROG_ENTRY;

int wimp_launch_exe(PROG_ENTRY entry);

int wimp_launch_exe(PROG_ENTRY entry)
{
	wimp_log("Launching: %s With args: %s\n", entry->path, entry->args);
	ShellExecute(NULL, "open", entry->path, entry->args, NULL, WIMP_EXE_WINDOW_SHOW);
	
	wimp_log("Closing: %s\n", entry->path);
	free(entry->path);
	free(entry->args);
	free(entry);
	return 0;
}

#endif

#ifdef __unix__

#include <unistd.h>
#include <sys/timeb.h>

typedef struct _PROG_ENTRY
{
	char* path;
	char** argv;
}* PROG_ENTRY;

int wimp_launch_lin(PROG_ENTRY entry);

int wimp_launch_lin(PROG_ENTRY entry)
{
	wimp_log("Launching: %s\n", entry->path);
	execv(entry->path, entry->argv);

	wimp_log("Closing: %s\n", entry->path);
	free(entry->path);
	free(entry->argv);
	free(entry);
	return 0;
}

#endif

int32_t wimp_start_library_process(const char* process_name, MAIN_FUNC_PTR main_func, WimpMainEntry entry)
{
	wimp_log("Starting %s!\n", process_name);
	PUThread* process_thread = p_uthread_create((PUThreadFunc)main_func, entry, false, process_name);
	if (process_thread == NULL)
	{
		wimp_log("Failed to create thread: %s", process_name);
		return WIMP_PROCESS_FAIL;
	}
	return WIMP_PROCESS_SUCCESS;
}

int32_t wimp_start_executable_process(const char* process_name, const char* executable, WimpMainEntry entry)
{
	//Get the directory of the running process
	char* path = malloc(MAX_DIRECTORY_PATH_LEN);
	if (path == NULL)
	{
		return WIMP_PROCESS_FAIL;
	}
	memset(path, 0, MAX_DIRECTORY_PATH_LEN);

	//Get the directory
#ifdef _WIN32
	GetModuleFileName(NULL, path, MAX_DIRECTORY_PATH_LEN);
#endif

#ifdef __unix__
	ssize_t linklen = readlink("/proc/self/exe", path, MAX_DIRECTORY_PATH_LEN);
	path[linklen] = '\0';
#endif

	printf("Current executable: %s\n", path);

	//Erase the file part from the string
	size_t current_dir_bytes = strlen(path) * sizeof(char);
	size_t last_slash_index = MAX_DIRECTORY_PATH_LEN;
	for (size_t i = current_dir_bytes; i > 0; --i)
	{
		if (path[i] == '/' || path[i] == '\\')
		{
			last_slash_index = i;
			break;
		}
	}

	if (last_slash_index == MAX_DIRECTORY_PATH_LEN)
	{
		wimp_log("Issue reading the path of the program! %s\n", path);
		return WIMP_PROCESS_FAIL;
	}

	//Blank everything after the index (except slash)
	memset(&path[last_slash_index + 1], 0, MAX_DIRECTORY_PATH_LEN - last_slash_index - 1);

	//Add the rest of the path specified - TODO allow ../../ format - currently can't!
	size_t pathlen = strlen(path) * sizeof(char);
	memcpy(&path[last_slash_index + 1], executable, pathlen);

	//If on windows, add ".exe"
#ifdef _WIN32
	const char exe_str = ".exe";
	memcpy(&path[strlen(path) * sizeof(char)], &exe_str, 4);
#endif

	//Check the file exists
	if (access(path, F_OK) != 0)
	{
		wimp_log("%s was not found!\n", path);
		return WIMP_PROCESS_FAIL;
	}

	//Make the entry args
	PROG_ENTRY prog_entry = malloc(sizeof(struct _PROG_ENTRY));
	if (prog_entry == NULL)
	{
		return WIMP_PROCESS_FAIL;
	}
	prog_entry->path = path; //Currently just refer to original

#ifdef _WIN32
	//For windows collate the other args
	size_t arglen = 0;

	for (int i = 0; i < entry->argc; ++i)
	{
		arglen += (strlen(entry->argv[i]) + 1) * sizeof(char); //+1 for spaces or null
	}
	
	prog_entry->args = malloc(arglen);
	if (prog_entry->args == NULL)
	{
		return WIMP_PROCESS_FAIL;
	}

	//Copy over the strings
	size_t offset = 0;
	for (int i = 0; i < entry->argc; ++i)
	{
		size_t strbytes = strlen(entry->argv[i]) * sizeof(char);
		memcpy(&prog_entry->args[offset], entry->argv[i], strbytes);
		offset += strbytes;
		
		//Add either a ' ' or '\0'
		if (i == entry->argc - 1)
		{
			prog_entry->args[offset] = '\0';
		}
		else
		{
			prog_entry->args[offset] = ' ';
		}
		offset++;
	}
#endif

#if __unix__
	prog_entry->path = path;
	prog_entry->argv = entry->argv;
#endif

	wimp_log("Running %s!\n", path);

#ifdef _WIN32
	PUThread* process_thread = p_uthread_create((PUThreadFunc)&wimp_launch_exe, prog_entry, false, process_name);
#endif

#ifdef __unix__
	PUThread* process_thread = p_uthread_create((PUThreadFunc)&wimp_launch_lin, prog_entry, false, process_name);
#endif

	if (process_thread == NULL)
	{
		wimp_log("Failed to create thread: %s", process_name);
		return WIMP_PROCESS_FAIL;
	}
	return WIMP_PROCESS_SUCCESS;
}



int32_t wimp_init()
{
	p_libsys_init();

#ifdef __unix__
	//Seed random for port assignment on linux - TODO get a better port assignment system
	struct timeb tb;
	ftime(&tb);
	srand(tb.time + (time_t)tb.millitm);
#endif
	return WIMP_PROCESS_SUCCESS;
}

void wimp_shutdown()
{
	p_libsys_shutdown();
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

int32_t wimp_assign_unused_local_port()
{
#ifdef _WIN32
	//bind dummy socket, get the port and return
	PSocket* reciever_socket;
    PSocketAddress* reciever_address;

    //Construct address for client, which should be listening
    reciever_address = p_socket_address_new("127.0.0.1", 0);
    if (reciever_address == NULL)
    {
		wimp_log("Failed to bind to unused port!\n");
		p_socket_address_free(reciever_address);
        return WIMP_PROCESS_FAIL;
    }

	if ((reciever_socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		wimp_log("Failed to bind to unused port!\n");
		p_socket_address_free(reciever_address);
		return WIMP_PROCESS_FAIL;
	}

	if (!p_socket_bind(reciever_socket, reciever_address, TRUE, NULL))
	{
		wimp_log("Failed to bind to unused port!\n");
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
#else
	//Linux seems to not play nice with the above method, use rand for now as it seems to be more consistent
	//TODO - please don't do this, or the above, but a third less sinister thing
	return rand() % (65535 + 1 - 49152) + 49152;
#endif
}

int32_t wimp_port_to_string(int32_t port, char* string_out)
{
	memset(string_out, 0, MAX_PORT_STRING_LEN);
	sprintf(string_out, "%d", port);
	return WIMP_PROCESS_SUCCESS;
}