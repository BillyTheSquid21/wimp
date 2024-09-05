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
	{ "RAW LONG MESSAGE", false, },
	{ "RESPONSE AWAITING", false},
	{ "PACKED LONG MESSAGE", false},
	{ "PACKED MULTIPLE LONG MESSAGES", false},
};

enum TEST_ENUMS
{
	RAW_LONG_MESSAGE,
	AWAITING_RESPONSE,
	PACKED_LONG_MESSAGE,
	PACKED_MULTIPLE_LONG_MESSAGES,
};

enum TEST_INSTRUCTIONS
{
	LONG_MESSAGE = 0,
	RESPONSE_AWAIT = 1,
	LONG_MESSAGE_PACKED = 2,
	LONG_MESSAGES_PACKED = 3,
};

const char* long_message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

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
	
	//1. Send the long message
	wimp_add_local_server("master", LONG_MESSAGE, long_message, ((strlen(long_message) + 1) * sizeof(char)));
	wimp_server_send_instructions(server);

	//2. Send response to await
	wimp_add_local_server("master", RESPONSE_AWAIT, NULL, 0);
	wimp_server_send_instructions(server);

	//3. Send the packed long message
	WimpStrPack pack2 = wimp_instr_pack_strings(1, long_message);
	wimp_add_local_server("master", LONG_MESSAGE_PACKED, pack2, pack2->pack_size);
	wimp_server_send_instructions(server);

	//4. Send the multiple packed long messages
	WimpStrPack pack3 = wimp_instr_pack_strings(4, long_message, long_message, long_message, long_message);
	wimp_add_local_server("master", LONG_MESSAGES_PACKED, pack3, pack3->pack_size);
	wimp_server_send_instructions(server);

	//Exit
	wimp_add_local_server("master", WIMP_INSTRUCTION_EXIT, NULL, 0);
	wimp_server_send_instructions(server);
	p_uthread_sleep(10000);

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
	wimp_start_library_process("test_process", (MAIN_FUNC_PTR)&client_main_lib_entry, P_UTHREAD_PRIORITY_HIGH, entry);

	//Start a local server for the master process
	wimp_init_local_server("master", "127.0.0.1", master_port);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the client process that the master started
	RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", end_process_port, &server->incomingmsg, &server->active);
	wimp_start_reciever_thread("test_process", "127.0.0.1", master_port, args);

	//Add the test process to the table for tracking
	wimp_process_table_add(&server->ptable, "test_process", "127.0.0.1", end_process_port, WIMP_Process_Child, NULL);

	//Accept the connection to the master->test_process reciever, started by the test_process
	wimp_server_process_accept(server, 1, "test_process");

	//This is a simple loop. 
	bool disconnect = false;
	while (!disconnect)
	{
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			if (wimp_instr_check(meta.instr, LONG_MESSAGE))
			{
				if (strcmp(meta.args, long_message) == 0)
				{
					PASS_MATRIX[RAW_LONG_MESSAGE].status = true;
				}
				wimp_instr_node_free(currentnode);
				disconnect = true;
				break;
			}
			currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		}
		wimp_instr_queue_high_prio_unlock(&server->incomingmsg);
		p_uthread_sleep(1);
	}

	//Await the response
	WimpInstrNode response = wimp_server_wait_response(server, RESPONSE_AWAIT, 0);
	if (response)
	{
		PASS_MATRIX[AWAITING_RESPONSE].status = true;
		wimp_instr_node_free(response);
	}

	disconnect = false;
	while (!disconnect)
	{
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			if (wimp_instr_check(meta.instr, LONG_MESSAGE_PACKED))
			{
				WimpStrPack pack = meta.args;
				if (strcmp(wimp_instr_pack_get_string(pack, 0), long_message) == 0)
				{
					PASS_MATRIX[PACKED_LONG_MESSAGE].status = true;
				}
			}
			else if (wimp_instr_check(meta.instr, LONG_MESSAGES_PACKED))
			{
				WimpStrPack pack = meta.args;
				bool success = true;
				for (int i = 0; i < 4; ++i)
				{
					char* str = wimp_instr_pack_get_string(pack, i);
					if (strcmp(str, long_message) != 0)
					{
						success = false;
						break;
					}
				}
				PASS_MATRIX[PACKED_MULTIPLE_LONG_MESSAGES].status = success;
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
		p_uthread_sleep(1);
	}

	//Cleanup
	wimp_log("Master thread closed\n");
	wimp_close_local_server();

	//Cleanup
	wimp_shutdown();

	wimp_test_validate_passmat(PASS_MATRIX, 4);
	return 0;
}