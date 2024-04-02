#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp_reciever.h>
#include <wimp_process.h>
#include <wimp_process_table.h>
#include <wimp_server.h>
#include <wimp_instruction.h>
#include <wimp_test.h>

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
	wimp_start_executable_process("test_process", "WIMP-Test-04_b", entry);

	//Start a local server for the master process
	wimp_init_local_server("master", "127.0.0.1", master_port);
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
			wimp_log("\nMaster recieved instruction:");
			DEBUG_WIMP_PRINT_INSTRUCTION_META(meta);

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
			else if (strcmp(meta.instr, WIMP_INSTRUCTION_LOG) == 0)
			{
				wimp_log("%s", meta.args);
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