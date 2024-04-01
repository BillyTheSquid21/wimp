#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp_reciever.h>
#include <wimp_process.h>
#include <wimp_process_table.h>
#include <wimp_server.h>
#include <wimp_instruction.h>
#include <wimp_test.h>
#include <wimp_data.h>

PASSMAT PASS_MATRIX[] =
{
	{ "PROCESS VALIDATION", false },
	{ "INSTRUCTION 1", false },
	{ "INSTRUCTION 2", false },
	{ "INSTRUCTION 3", false },
	{ "INSTRUCTION 4", false }
};

enum TEST_ENUMS
{
	STEP_PROCESS_VALIDATION,
	STEP_INSTRUCTION_1,
	STEP_INSTRUCTION_2,
	STEP_INSTRUCTION_3,
	STEP_INSTRUCTION_4,
};

/*
* This is an example client main. It takes the domains and ports as cmd arguments and creates and starts a server.
* After sending the commands, the server closes.
*/
int client_main_entry(int argc, char** argv)
{
	wimp_log("Test process!\n");

	int32_t writebuffer = 0;

	//Default this domain and port
	const char* process_domain = "127.0.0.1";
	int32_t process_port = 8001;

	//Default the master domain and port
	const char* master_domain = "127.0.0.1";
	int32_t master_port = 8000;

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
	}

	//Create a server local to this thread
	wimp_init_local_server("test_process", "127.0.0.1", process_port, NULL);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the master process that called this thread
	RecieverArgs args = wimp_get_reciever_args("test_process", master_domain, master_port, &server->incomingmsg);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Add the master process to the table for tracking
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", WIMP_Process_Parent, master_port, NULL);

	//Accept the connection to the test_process->master reciever, started by the master thread
	wimp_server_process_accept(server, 1, "master");
	p_uthread_sleep(100);

	//This server won't loop, so send some instructions to the master thread
	
	//Instruction 1 - This sends a simple instr that the master will ignore. It has no additional arguments
	wimp_add_local_server("master", "blank_instr", NULL, 0);

	//Instruction 2 - This sends a simple instr that tells the master to say hello. It has no additional arguments
	wimp_add_local_server("master", "say_hello", NULL, 0);

	//Instruction 3 - This sends a more complex instr, that tells the master to echo the string sent.
	const char* echo_string = "Echo!";
	wimp_add_local_server("master", "echo", echo_string, (strlen(echo_string) + 1) * sizeof(char));

	//Instruction 4 - This simple tells the master to exit - TODO: This implementation may change in the future
	wimp_add_local_server("master", "exit", NULL, 0);

	//This tells the server to send off the instructions
	wimp_server_send_instructions(server);
	p_uthread_sleep(1000);

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
	int32_t end_process_port = wimp_assign_unused_local_port();

	//The ports are converted to strings for use as command line arguments
	WimpPortStr port_string;
	wimp_port_to_string(end_process_port, port_string);

	WimpPortStr master_port_string;
	wimp_port_to_string(master_port, master_port_string);

	//Start the client process, creating the command line arguments and creating a new thread
	WimpMainEntry entry = wimp_get_entry(4, "--master-port", master_port_string, "--process-port", port_string);
	wimp_start_library_process("test_process", (MAIN_FUNC_PTR)&client_main_lib_entry, P_UTHREAD_PRIORITY_LOW, entry);

	//Start a local server for the master process
	wimp_init_local_server("master", "127.0.0.1", master_port, NULL);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the client process that the master started
	RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", end_process_port, &server->incomingmsg);
	wimp_start_reciever_thread("test_process", "127.0.0.1", master_port, args);

	//Add the test process to the table for tracking
	wimp_process_table_add(&server->ptable, "test_process", "127.0.0.1", end_process_port, WIMP_Process_Child, NULL);

	//Accept the connection to the master->test_process reciever, started by the test_process
	wimp_server_process_accept(server, 1, "test_process");

	//Validate that the process correctly started. Sends a ping packet to make sure is listening
	if (wimp_server_check_process_listening(server, "test_process"))
	{
		wimp_log("Process validated!\n");
		PASS_MATRIX[STEP_PROCESS_VALIDATION].status = true;
	}

	//This is a simple loop. 
	bool disconnect = false;
	while (!disconnect)
	{
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			//wimp_log("\nMaster recieved instruction:");
			//DEBUG_WIMP_PRINT_INSTRUCTION_META(meta);

			if (strcmp(meta.instr, "blank_instr") == 0)
			{
				wimp_log("\n");
				PASS_MATRIX[STEP_INSTRUCTION_1].status = true;
			}
			else if (strcmp(meta.instr, "say_hello") == 0)
			{
				wimp_log("HELLO!\n");
				PASS_MATRIX[STEP_INSTRUCTION_2].status = true;
			}
			else if (strcmp(meta.instr, "echo") == 0)
			{
				//Get the arguments
				const char* echo_string = (const char*)meta.args;
				wimp_log("%s\n", echo_string);

				if (strcmp(echo_string, "Echo!") == 0)
				{
					PASS_MATRIX[STEP_INSTRUCTION_3].status = true;
				}
			}
			else if (strcmp(meta.instr, WIMP_INSTRUCTION_EXIT) == 0)
			{
				wimp_log("\n");
				PASS_MATRIX[STEP_INSTRUCTION_4].status = true;
				disconnect = true;
			}

			wimp_instr_node_free(currentnode);
			currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		}
		wimp_instr_queue_high_prio_unlock(&server->incomingmsg);
	}

	//Cleanup
	wimp_log("Master thread closed\n");
	wimp_close_local_server();

	//Cleanup
	wimp_shutdown();

	wimp_test_validate_passmat(PASS_MATRIX, 5);
	return 0;
}