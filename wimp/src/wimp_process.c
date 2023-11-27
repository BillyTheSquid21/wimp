#include <wimp_process.h>

//Starts the reciever and keeps track of it
int32_t wimp_start_reciever_process(const char* process_name, const char* domain, int32_t port_number, uint8_t* writebuff);

/*
* The thread started from here is responsible for itself, is essentially fire and forget
*/
int32_t wimp_start_library_process(const char* process_name, MAIN_FUNC_PTR main_func, const char* domain, int32_t port_number,  uint8_t* writebuff)
{
	PUThread* process_thread = p_uthread_create(main_func, NULL, false, process_name);
	if (process_thread == NULL)
	{
		printf("Failed to create thread: %s", process_name);
		return WIMP_PROCESS_FAIL;
	}

	if (wimp_start_reciever_process(process_name, domain, port_number, writebuff) == WIMP_PROCESS_FAIL)
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

	args->domain = domain; args->port_number = port_number; args->writebuff = writebuff;

	PUThread* reciever_thread = p_uthread_create(reciever_func, (ppointer)args, false, "reciever");
	return WIMP_PROCESS_SUCCESS;
}