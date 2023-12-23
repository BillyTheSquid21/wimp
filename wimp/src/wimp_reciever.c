#include <wimp_reciever.h>

RecieverArgs wimp_get_reciever_args(const char* recfrom_domain, int32_t recfrom_port, const char* process_domain, int32_t process_port, uint8_t* wrtbuff)
{
	RecieverArgs recargs = malloc(sizeof(struct _RecieverArgs));
	if (recargs == NULL)
	{
		return NULL;
	}

	//Assume all the strings provided are valid
	size_t recfrom_bytes = (strlen(recfrom_domain) + 1) * sizeof(char);
	size_t process_bytes = (strlen(process_domain) + 1) * sizeof(char);
	recargs->recfrom_domain = malloc(recfrom_bytes);
	recargs->process_domain = malloc(process_bytes);

	if (recargs->recfrom_domain == NULL || recargs->process_domain == NULL)
	{
		return NULL;
	}

	memcpy(recargs->recfrom_domain, recfrom_domain, recfrom_bytes);
	memcpy(recargs->process_domain, process_domain, process_bytes);

	recargs->process_port = process_port;
	recargs->recfrom_port = recfrom_port;
	recargs->writebuff = wrtbuff;
	return recargs;
}

void wimp_free_reciever_args(RecieverArgs args)
{
	free(args->process_domain);
	free(args->recfrom_domain);
	free(args);
}

void wimp_reciever_recieve(RecieverArgs args)
{
	printf("HELLO RECIEVER! %s, %s\n", args->process_domain, args->recfrom_domain);
	wimp_free_reciever_args(args);
	return;
}

int32_t wimp_start_reciever_thread(const char* reciever_name, RecieverArgs args)
{
	printf("Starting Reciever: %s!\n", reciever_name);
	PUThread* process_thread = p_uthread_create(&wimp_reciever_recieve, args, false, reciever_name);
	if (process_thread == NULL)
	{
		printf("Failed to create thread: %s", reciever_name);
		return WIMP_RECIEVER_FAIL;
	}
	return WIMP_RECIEVER_SUCCESS;
}