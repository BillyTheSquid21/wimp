#include <wimp.h>

int client_main(int argc, char** argv)
{
	//Default values in case no arguments supplied
	const char* process_domain = "127.0.0.1";
	int32_t process_port = 8001;
	const char* master_domain = "127.0.0.1";
	int32_t master_port = 8000;

	//Read the args in the C way
	//We also check i + 1 < argc to ensure next argument was specified
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "--master-port") == 0 && i + 1 < argc)
		{
			master_port = strtol(argv[i+1], NULL, 10);
		}
		else if (strcmp(argv[i], "--process-port") == 0 && i + 1 < argc)
		{	
			process_port = strtol(argv[i+1], NULL, 10);
		}
	}

	//Same old initialization
	if (wimp_init() != WIMP_PROCESS_SUCCESS)
	{
		return -1;
	}

	if (wimp_init_local_server("client", "127.0.0.1", process_port) != WIMP_SERVER_SUCCESS)
	{
		printf("Failed to init the client local server!!!\n");
		wimp_shutdown();
		return -2;
	}
	printf("WIMP client succeeded on port: %d\n", process_port);

	//Get the client local server
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread to recieve from the master
	RecieverArgs args = wimp_get_reciever_args("client", "127.0.0.1", master_port, &server->incomingmsg, &server->active);
	if (wimp_start_reciever_thread("master", "127.0.0.1", process_port, args) != WIMP_RECIEVER_SUCCESS)
	{
		printf("Failed to start client reciever thread!\n");
		wimp_close_local_server();
		wimp_shutdown();
		return -4;
	}

	//Add the master process to the server process table
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, WIMP_Process_Parent, NULL);

	//Accept the incoming connection from the master reciever
	//This is the reciever that writes to the master incoming queue
	if (wimp_server_process_accept(server, 1, "master") != WIMP_SERVER_SUCCESS)
	{
		printf("Failed to accept connection from master reciever!\n");
		wimp_close_local_server();
		wimp_shutdown();
		return -5;
	}

	printf("Successfully linked to master!\n");
	p_uthread_sleep(1000);

	//Add instructions
	wimp_add_local_server("master", "blank_instr", NULL, 0);
	wimp_add_local_server("master", "say_hello", NULL, 0);

	const char* echo_string = "Echo!";
	wimp_add_local_server("master", "echo", echo_string, (strlen(echo_string) + 1) * sizeof(char));
	wimp_log_success("Sent instructions!\n");
	wimp_add_local_server("master", WIMP_INSTRUCTION_EXIT, NULL, 0);
	wimp_server_send_instructions(server);
	p_uthread_sleep(1000);

	wimp_close_local_server();
	wimp_shutdown();
	return 0;
}

int client_main_entry(WimpMainEntry entry)
{
	int res = client_main(entry->argc, entry->argv);
	wimp_free_entry(entry);
	return res;
}

int main(void)
{
	if (wimp_init() != WIMP_PROCESS_SUCCESS)
	{
		printf("WIMP failed :(");
		return -1;
	}

	//Get the master port and init local server
	int32_t master_port = wimp_assign_unused_local_port();
	if (wimp_init_local_server("master", "127.0.0.1", master_port) != WIMP_SERVER_SUCCESS)
	{
		printf("Failed to init the local server!!!\n");
		wimp_shutdown();
		return -2;
	}
	printf("WIMP succeeded on port: %d\n", master_port);

	//Get the client port and convert both ports to strings
	int32_t process_port = wimp_assign_unused_local_port();
	WimpPortStr port_string;
	WimpPortStr master_port_string;
	wimp_port_to_string(process_port, port_string);
	wimp_port_to_string(master_port, master_port_string);

	//Start the client process
	WimpMainEntry entry = wimp_get_entry(4, "--master-port", master_port_string, "--process-port", port_string);
	if (wimp_start_library_process("client", (MAIN_FUNC_PTR)&client_main_entry, P_UTHREAD_PRIORITY_NORMAL, entry) != WIMP_PROCESS_SUCCESS)
	{
		printf("Failed to start library process!\n");
		wimp_close_local_server();
		wimp_shutdown();
		return -3;
	}

	//Get the local server pointer
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread to recieve from our client
	RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", process_port, &server->incomingmsg, &server->active);
	if (wimp_start_reciever_thread("client", "127.0.0.1", master_port, args) != WIMP_RECIEVER_SUCCESS)
	{
		printf("Failed to start reciever thread!\n");
		wimp_close_local_server();
		wimp_shutdown();
		return -4;
	}

	//Add the client process to the server process table
	wimp_process_table_add(&server->ptable, "client", "127.0.0.1", process_port, WIMP_Process_Child, NULL);

	//Accept the incoming connection from the client reciever
	//This is the reciever that writes to the client incoming queue
	if (wimp_server_process_accept(server, 1, "client") != WIMP_SERVER_SUCCESS)
	{
		printf("Failed to accept connection from client reciever!\n");
		wimp_close_local_server();
		wimp_shutdown();
		return -5;
	}

	printf("Successfully linked to client!\n");
	p_uthread_sleep(1000);

	//Simple loop
	bool disconnect = false;
	while (!disconnect)
	{
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			if (wimp_instr_check(meta.instr, "blank_instr"))
			{
				wimp_log("\n");
			}
			else if (wimp_instr_check(meta.instr, "say_hello"))
			{
				wimp_log("HELLO!\n");
			}
			else if (wimp_instr_check(meta.instr, "echo"))
			{
				//Get the arguments
				const char* echo_string = (const char*)meta.args;
				wimp_log("%s\n", echo_string);
			}
			else if (wimp_instr_check(meta.instr, WIMP_INSTRUCTION_LOG))
			{
				wimp_log(meta.args);
			}
			else if (wimp_instr_check(meta.instr, WIMP_INSTRUCTION_EXIT))
			{
				wimp_log("\n");
				disconnect = true;
			}

			wimp_instr_node_free(currentnode);
			currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		}
		wimp_instr_queue_high_prio_unlock(&server->incomingmsg);
	}

	wimp_close_local_server();
	wimp_shutdown();
	return 0;
}