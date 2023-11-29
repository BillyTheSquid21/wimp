#include <wimp_process.h>

//Starts the reciever and keeps track of it
int32_t wimp_start_reciever_process(const char* process_name, const char* domain, int32_t port_number, uint8_t* writebuff);

/*
* The thread started from here is responsible for itself, is essentially fire and forget
*/
int32_t wimp_start_library_process(const char* process_name, MAIN_FUNC_PTR main_func, const char* domain, int32_t master_port, int32_t end_port, uint8_t* writebuff)
{
	//argc argv system with arg --port and --master-port
	//TODO - make so can append more args
	//Have the null returns to make intellisense shut up
	MainEntry* main_entry = malloc(sizeof(MainEntry));
	if (main_entry == NULL)
	{
		return WIMP_PROCESS_FAIL;
	}

	main_entry->argc = 4;
	main_entry->argv = malloc(4 * sizeof(char*));
	if (main_entry->argv == NULL)
	{
		return WIMP_PROCESS_FAIL;
	}

	main_entry->argv[0] = malloc(7 * sizeof(char));
	main_entry->argv[1] = malloc(2 * sizeof(char));
	main_entry->argv[2] = malloc(14 * sizeof(char));
	main_entry->argv[3] = malloc(2 * sizeof(char));
	for (int i = 0; i < main_entry->argc; ++i)
	{
		if (main_entry->argv[i] == NULL)
		{
			return WIMP_PROCESS_FAIL;
		}
	}

	strcpy(main_entry->argv[0], "--port");
	sprintf(main_entry->argv[1], "%d", end_port);
	strcpy(main_entry->argv[2], "--master-port");
	sprintf(main_entry->argv[3], "%d", master_port);

	PUThread* process_thread = p_uthread_create(main_func, main_entry, false, process_name);
	if (process_thread == NULL)
	{
		printf("Failed to create thread: %s", process_name);
		return WIMP_PROCESS_FAIL;
	}

	if (wimp_start_reciever_process(process_name, domain, end_port, writebuff) == WIMP_PROCESS_FAIL)
	{
		printf("Failed to create reciever thread: %s", process_name);
		return WIMP_RECIEVER_FAIL;
	}

	return WIMP_PROCESS_SUCCESS;
}

int32_t wimp_start_reciever_process(const char* process_name, const char* domain, int32_t port_number, uint8_t* writebuff)
{
	if (process_name == NULL)
	{
		return WIMP_PROCESS_FAIL;
	}

	RECIEVER_FUNC_PTR reciever_func = &wimp_reciever_recieve;
	//size_t process_name_length = sizeof(process_name);
	//char* reciever_name = malloc(process_name_length + 2);
	//if (reciever_name == NULL)
	//{
	//	return WIMP_PROCESS_FAIL;
	//}

	//TODO - figure out if this is dumb - the strings i dont get yet
	//strcpy(reciever_name, process_name); //Due to check previously, ignore warning
	//strcpy(reciever_name-3, "_r");
	
	//The arguments will be freed in the receiver thread so do use malloc
	//Is done as would otherwise go out of scope after the p_uthread_create
	RecieverArgs* args = malloc(sizeof(RecieverArgs));
	if (args == NULL)
	{
		return WIMP_PROCESS_FAIL;
	}

	args->domain = domain; args->target_port_number = port_number; args->writebuff = writebuff;

	PUThread* reciever_thread = p_uthread_create(reciever_func, (ppointer)args, false, "reciever");
	return WIMP_PROCESS_SUCCESS;
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
        return 30000; //Random return value - shouldn't happen
    }

	if ((reciever_socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		p_socket_address_free(reciever_address);
		return 30000;
	}

	if (!p_socket_bind(reciever_socket, reciever_address, FALSE, NULL))
	{
		p_socket_address_free(reciever_address);
		p_socket_free(reciever_socket);
		return 30000;
	}

	PSocketAddress* bound_address = p_socket_get_local_address(reciever_socket, NULL);
	
	int32_t port = p_socket_address_get_port(bound_address);
	p_socket_address_free(reciever_address);
	p_socket_address_free(bound_address);
    p_socket_close(reciever_socket, NULL);
	return port;
}