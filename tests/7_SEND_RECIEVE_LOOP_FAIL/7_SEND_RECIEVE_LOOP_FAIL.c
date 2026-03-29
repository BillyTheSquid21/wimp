#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp.h>
#include <wimp_test.h>

PASSMAT PASS_MATRIX[] =
{
	{ "FAIL_SECOND_LOCAL_SERVER", false },
	{ "FAIL_CLOSE_SECOND_SERVER", false },
	{ "FAIL_LOCAL_SERVER_ADDRESS", false },
	{ "FAIL_IDENTICAL_ADDRESS", false },
	{ "SUCCESS_PROCESS_VALIDATION", false },
};

enum TEST_ENUMS
{
	FAIL_SECOND_LOCAL_SERVER = 0,
	FAIL_CLOSE_SECOND_SERVER = 1,
	FAIL_LOCAL_SERVER_ADDRESS = 2,
	FAIL_IDENTICAL_ADDRESS = 3,
	SUCCESS_PROCESS_VALIDATION = 4,
};

enum TEST_INSTRUCTIONS
{
	BLANK_INSTR = 0,
	SAY_HELLO = 1,
	ECHO = 2,
};

/*
* This main function is deliberately designed to break and fail to test the negative paths
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
	wimp_init_local_server("test_process", "127.0.0.1", process_port);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the master process that called this thread
	RecieverArgs args = wimp_get_reciever_args("test_process", master_domain, master_port, &server->incomingmsg, &server->active);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Add the master process to the table for tracking
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, WIMP_Process_Parent, NULL);

	//Accept the connection to the test_process->master reciever, started by the master thread
	wimp_server_process_accept(server, 1, "master");
	p_uthread_sleep(100);

	//This server won't loop, so send some instructions to the master thread
	
	//Instruction 1 - This sends a simple instr that the master will ignore. It has no additional arguments
	wimp_add_local_server("master", BLANK_INSTR, NULL, 0);

	//Instruction 2 - This sends a simple instr that tells the master to say hello. It has no additional arguments
	wimp_add_local_server("master", SAY_HELLO, NULL, 0);

	//Instruction 3 - This sends a more complex instr, that tells the master to echo the string sent.
	const char* echo_string = "Echo!";
	wimp_add_local_server("master", ECHO, echo_string, (strlen(echo_string) + 1) * sizeof(char));

	//Instruction 4 - This simple tells the master to exit
	wimp_add_local_server("master", WIMP_INSTRUCTION_EXIT, NULL, 0);

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

int invalid_main_entry(int argc, char** argv)
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
			master_port = strtol(argv[i + 1], NULL, 10);
		}
		else if (strcmp(argv[i], "--process-port") == 0 && i + 1 < argc)
		{
			process_port = strtol(argv[i + 1], NULL, 10);
		}
	}

	//Create a server local to this thread
	wimp_init_local_server("invalid_process", "127.0.0.1", process_port);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the master process that called this thread
	RecieverArgs args = wimp_get_reciever_args("test_process", master_domain, master_port, &server->incomingmsg, &server->active);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Add the master process to the table for tracking
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, WIMP_Process_Parent, NULL);

	//Accept the connection to the test_process->master reciever, started by the master thread
	wimp_server_process_accept(server, 1, "master");
	p_uthread_sleep(100);

	//This server won't loop, so send some instructions to the master 
	//Send the hashed ping to cover this case
	wimp_add_local_server("master", wimp_instr("PING"), NULL, 0);

	//Instruction 1 - This sends a simple instr that the master will ignore. It has no additional arguments
	//This instruction is produced from the hash of the string "BLANK_INSTR" so check it works.
	wimp_add_local_server("master", BLANK_INSTR, NULL, 0);

	//Instruction 2 - This sends a simple instr that tells the master to say hello. It has no additional arguments
	wimp_add_local_server("master", SAY_HELLO, NULL, 0);

	//Instruction 3 - This sends a more complex instr, that tells the master to echo the string sent.
	const char* echo_string = "Echo!";
	wimp_add_local_server("master", ECHO, echo_string, (strlen(echo_string) + 1) * sizeof(char));

	//Instruction 4 - This simple tells the master to exit
	wimp_add_local_server("master", WIMP_INSTRUCTION_EXIT, NULL, 0);

	//This tells the server to send off the instructions
	wimp_server_send_instructions(server);
	p_uthread_sleep(1000);

	//This should also shut down the reciever
	wimp_log("Client thread closed\n");
	wimp_close_local_server();

	return 0;
}

int invalid_main_lib_entry(WimpMainEntry entry)
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

	//Start a local server for the master process with an invalid IP - this should fail
	int32_t result = wimp_init_local_server("master", "notanip", master_port);
	if (result == WIMP_SERVER_ADDRESS_FAIL)
	{
		PASS_MATRIX[FAIL_LOCAL_SERVER_ADDRESS].status = true;
	}

	//Start the local server with the correct details - this should succeed
	wimp_init_local_server("master", "127.0.0.1", master_port);
	WimpServer* server = wimp_get_local_server();

	//Try to create a second server (not through local server) with the same details - this should fail as the port is already in use
	WimpServer* server2 = malloc(sizeof(WimpServer));
	result = wimp_create_server(server2, "master2", "127.0.0.1", master_port);
	if (result == WIMP_SERVER_BIND_FAIL)
	{
		PASS_MATRIX[FAIL_IDENTICAL_ADDRESS].status = true;
	}

	//Try to start a second local server - this should fail
	result = wimp_init_local_server("master", "127.0.0.1", master_port);
	if (result == WIMP_SERVER_FAIL)
	{
		PASS_MATRIX[FAIL_SECOND_LOCAL_SERVER].status = true;
	}

	//The ports are converted to strings for use as command line arguments
	WimpPortStr port_string;
	wimp_port_to_string(end_process_port, port_string);

	WimpPortStr master_port_string;
	wimp_port_to_string(master_port, master_port_string);

	//Start the client process, creating the command line arguments and creating a new thread
	WimpMainEntry entry = wimp_get_entry(4, "--master-port", master_port_string, "--process-port", port_string);
	wimp_start_library_process("test_process", (MAIN_FUNC_PTR)&client_main_lib_entry, P_UTHREAD_PRIORITY_LOW, entry);

	//Start a reciever thread for the client process that the master started
	result = wimp_start_local_server_reciever_thread("test_process", "127.0.0.1", end_process_port);

	//Add the test process to the table for tracking
	wimp_add_local_server_process("test_process", "127.0.0.1", end_process_port, WIMP_Process_Child);

	//Validate that the process correctly started. Sends a ping packet to make sure is listening
	if (wimp_server_check_process_listening(server, "test_process"))
	{
		wimp_log("Process validated!\n");
		PASS_MATRIX[SUCCESS_PROCESS_VALIDATION].status = true;
	}

	//Wait for instructions to pile up
	p_uthread_sleep(1000);

	//This is a simple loop. 
	bool disconnect = false;
	while (!disconnect)
	{
		wimp_incoming_queue_local_server_lock();
		wimp_log("Blank Instr count: %d\n", wimp_instr_get_instruction_count(&server->incomingmsg, BLANK_INSTR));
		WimpInstrNode currentnode = wimp_incoming_queue_local_server_pop();
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			switch (meta.instr)
			{
			case BLANK_INSTR:
				wimp_log("\n");
				//PASS_MATRIX[STEP_INSTRUCTION_1].status = true;
				break;
			case SAY_HELLO:
				wimp_log("HELLO!\n");
				//PASS_MATRIX[STEP_INSTRUCTION_2].status = true;
				break;
			case ECHO:
				//Get the arguments
				const char* echo_string = (const char*)meta.args;
				wimp_log("%s\n", echo_string);

				if (strcmp(echo_string, "Echo!") == 0)
				{
					//PASS_MATRIX[STEP_INSTRUCTION_3].status = true;
				}
				break;
			case WIMP_INSTRUCTION_EXIT:
				wimp_log("\n");
				//PASS_MATRIX[STEP_INSTRUCTION_4].status = true;
				disconnect = true;
				break;
			case WIMP_INSTRUCTION_LOG:
				//Get the arguments
				const char* log_string = (const char*)meta.args;
				wimp_log("%s\n", log_string);
				break;
			default:
				wimp_log_important("Unexpected instruction recieved %d\n", meta.instr);
				break;
			}

			wimp_instr_node_free(currentnode);
			currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		}
		wimp_incoming_queue_local_server_unlock();
	}

	//Cleanup
	wimp_log("Master thread closed\n");
	wimp_close_local_server();

	//Attemp to close the local server again - this should fail as it is already closed
	//Check after as will segfault if it tries to close an already closed server
	wimp_close_local_server();
	if (wimp_get_local_server() == NULL)
	{
		PASS_MATRIX[FAIL_CLOSE_SECOND_SERVER].status = true;
	}

	//Cleanup
	wimp_shutdown();

	bool passed = wimp_test_validate_passmat(PASS_MATRIX, sizeof(PASS_MATRIX)/sizeof(PASSMAT));
	return passed ? 0 : 1;
}