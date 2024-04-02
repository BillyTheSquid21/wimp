#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp_reciever.h>
#include <wimp_process.h>
#include <wimp_process_table.h>
#include <wimp_server.h>
#include <wimp_instruction.h>
#include <wimp_test.h>
#include <wimp_log.h>

PASSMAT PASS_MATRIX[] =
{
	{ "PROCESS VALIDATION", false, },
	{ "CHILD 1 RECEIVES FROM CHILD 2", false, },
	{ "CHILD 2 RECEIVES FROM CHILD 1", false, },
};

enum TEST_ENUMS
{
	STEP_PROCESS_VALIDATION,
	C1_RECIEVESFROM_C2,
	C2_RECIEVESFROM_C1,
};

/*
* This is an example client main. It takes the domains and ports as cmd arguments and creates and starts a server.
* After sending the commands, the server closes.
*/
int client_main_entry(int argc, char** argv)
{
	int32_t writebuffer = 0;

	//Default this domain and port
	const char* process_domain = "127.0.0.1";
	int32_t process_port = 8001;

	//Default the master domain and port
	const char* master_domain = "127.0.0.1";
	int32_t master_port = 8000;

	char* process_name = "client";

	//Read the args, look for the --master and --proc args
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
		else if (strcmp(argv[i], "--process-name") == 0 && i + 1 < argc)
		{
			process_name = argv[i+1];
		}
	}

	wimp_log("Client: %s\n", process_name);

	//Create a server local to this thread
	wimp_init_local_server(process_name, "127.0.0.1", process_port);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the master process that called this thread
	RecieverArgs args = wimp_get_reciever_args(process_name, master_domain, master_port, &server->incomingmsg);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Add the master process to the table for tracking
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, WIMP_Process_Parent, NULL);

	//Accept the connection to the test_process->master reciever, started by the master thread
	wimp_server_process_accept(server, 1, "master");

	//Step 1. Send an instruction to the other client
	char* other_client = NULL;
	if (strcmp(process_name, "client1") == 0)
	{
		other_client = "client2";
	}
	else
	{
		other_client = "client1";
	}
	wimp_log("Other client: %s\n", other_client);

	//Send the instr here
	wimp_add_local_server(other_client, "say_hello", NULL, 0);
	wimp_add_local_server(other_client, "exit", NULL, 0);
	wimp_server_send_instructions(server);

	//Program loop here
	bool disconnect = false;
	while (!disconnect)
	{
		//Read incoming
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			if (strcmp(meta.instr, "say_hello") == 0)
			{
				wimp_log("Hello! Recieved from: %s\n", meta.source_process);
				if (strcmp(meta.source_process, other_client) == 0)
				{
					if (strcmp(meta.dest_process, "client1") == 0)
					{
						PASS_MATRIX[C1_RECIEVESFROM_C2].status = true;
					}
					else if (strcmp(meta.dest_process, "client2") == 0)
					{
						PASS_MATRIX[C2_RECIEVESFROM_C1].status = true;
					} 
				}
			}
			else if (strcmp(meta.instr, WIMP_INSTRUCTION_EXIT) == 0)
			{
				wimp_log("Exiting!\n");
				disconnect = true;
			}
			wimp_instr_node_free(currentnode);
			currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		}
		wimp_instr_queue_high_prio_unlock(&server->incomingmsg);

		//Send outgoing
		wimp_server_send_instructions(server);
	}

	p_uthread_sleep(1000);

	//Will get two arriving but this shouldn't matter
	wimp_add_local_server("master", "exit", NULL, 0);

	//This should also shut down the reciever
	wimp_log("Client thread closed\n");
	wimp_close_local_server();

	return 0;
}

/*
* This is the entry for the lib. For this test, as will not be started from a separate executable this will be the only entry point used.
*/
int client_main_lib_entry(WimpMainEntry entry)
{
	int res = client_main_entry(entry->argc, entry->argv);
	wimp_free_entry(entry);
	return res;
}

/*
* This is the main master thread. 
*/
int main(void) 
{

	//Initialize the socket library
	wimp_init();

	//Get unused random ports for the master and end process to run on
	int32_t master_port = wimp_assign_unused_local_port();
	int32_t client1_port = wimp_assign_unused_local_port();
	int32_t client2_port = wimp_assign_unused_local_port();

	//The ports are converted to strings for use as command line arguments
	WimpPortStr client1_port_string;
	wimp_port_to_string(client1_port, client1_port_string);

	WimpPortStr client2_port_string;
	wimp_port_to_string(client2_port, client2_port_string);

	WimpPortStr master_port_string;
	wimp_port_to_string(master_port, master_port_string);

	//Start the client processes, creating the command line arguments and creating a new thread
	WimpMainEntry entry = wimp_get_entry(6, "--master-port", master_port_string, "--process-port", client1_port_string, "--process-name", "client1");
	wimp_start_library_process("client1", (MAIN_FUNC_PTR)&client_main_lib_entry, P_UTHREAD_PRIORITY_LOW, entry);

	entry = wimp_get_entry(6, "--master-port", master_port_string, "--process-port", client2_port_string, "--process-name", "client2");
	wimp_start_library_process("client2", (MAIN_FUNC_PTR)&client_main_lib_entry, P_UTHREAD_PRIORITY_LOW, entry);

	//Start a local server for the master process
	wimp_init_local_server("master", "127.0.0.1", master_port);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the client processes that the master started
	RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", client1_port, &server->incomingmsg);
	wimp_start_reciever_thread("client1", "127.0.0.1", master_port, args);

	args = wimp_get_reciever_args("master", "127.0.0.1", client2_port, &server->incomingmsg);
	wimp_start_reciever_thread("client2", "127.0.0.1", master_port, args);

	//Add the test process to the table for tracking
	wimp_process_table_add(&server->ptable, "client1", "127.0.0.1", client1_port, WIMP_Process_Child, NULL);
	wimp_process_table_add(&server->ptable, "client2", "127.0.0.1", client2_port, WIMP_Process_Child, NULL);

	//Accept the connection to the master->test_process reciever, started by the test_process
	wimp_server_process_accept(server, 2, "client1", "client2");

	//Validate that the process correctly started. Sends a ping packet to make sure is listening
	if (wimp_server_check_process_listening(server, "client1") && wimp_server_check_process_listening(server, "client2"))
	{
		wimp_log("Processes validated!\n");
		PASS_MATRIX[STEP_PROCESS_VALIDATION].status = true;
	}
	else
	{
		wimp_log("Processes couldn't be validated!\n");
	}

	//Perform the two loops - read incoming, then send outgoing
	bool disconnect = false;
	while (!disconnect)
	{
		//Read incoming
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			//DEBUG_WIMP_PRINT_INSTRUCTION_META(meta);
			if (wimp_server_instr_routed(server, meta.dest_process, currentnode))
			{
				//Add to the outgoing and continue to prevent freeing
				currentnode = wimp_instr_queue_pop(&server->incomingmsg);
				continue;
			}
			else if (strcmp(meta.instr, WIMP_INSTRUCTION_LOG) == 0)
			{
				wimp_log("%s", meta.args);
			}
			else if (strcmp(meta.instr, WIMP_INSTRUCTION_EXIT) == 0)
			{
				wimp_log("Exiting!\n");
				disconnect = true;
			}
			wimp_instr_node_free(currentnode);
			currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		}
		wimp_instr_queue_high_prio_unlock(&server->incomingmsg);

		//Send outgoing
		wimp_server_send_instructions(server);
	}

	p_uthread_sleep(1000);

	//Cleanup
	wimp_log("Master thread closed\n");
	wimp_close_local_server();

	//Cleanup
	wimp_shutdown();

	wimp_test_validate_passmat(PASS_MATRIX, 3);
	return 0;
}