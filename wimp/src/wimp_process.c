#include <wimp_process.h>
#include <wimp_log.h>

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

	//Process the command line args supplied
	main_entry->argc = argc;
	main_entry->argv = malloc((size_t)argc * sizeof(char*));
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
			free(main_entry);
			va_end(argp);
			return NULL;
		}
		main_entry->argv[i] = argv;
		memcpy(main_entry->argv[i], arg, arg_bytes);
	}
	va_end(argp);
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
	//bind dummy socket, get the port and return
	PSocket* reciever_socket;
    PSocketAddress* reciever_address;

    //Construct address for client, which should be listening
    reciever_address = p_socket_address_new("127.0.0.1", 0);
    if (reciever_address == NULL)
    {
		p_socket_address_free(reciever_address);
        return WIMP_PROCESS_FAIL;
    }

	if ((reciever_socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		p_socket_address_free(reciever_address);
		return WIMP_PROCESS_FAIL;
	}

	if (!p_socket_bind(reciever_socket, reciever_address, FALSE, NULL))
	{
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

int32_t wimp_port_to_string(int32_t port, char* string_out)
{
	memset(string_out, 0, MAX_PORT_STRING_LEN);
	sprintf(string_out, "%d", port);
	return WIMP_PROCESS_SUCCESS;
}