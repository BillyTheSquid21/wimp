#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <utility/sds.h>
#include <wimp_reciever.h>
#include <wimp_process.h>
#include <wimp_process_table.h>
#include <wimp_server.h>
#include <wimp_instruction.h>
#include <wimp_test.h>
#include <wimp_data.h>

PASSMAT PASS_MATRIX[] =
{
	{ "DATA INIT", false },
	{ "DATA RESERVE", false },
	{ "CHILD1 LINK TABLE", false },
	{ "CHILD1 LINK DATA", false },
	{ "CHILD1 READ DATA", false },
	{ "CHILD1 WRITE DATA", false },
	{ "CHILD2 LINK TABLE", false },
	{ "CHILD2 LINK DATA", false },
	{ "CHILD2 READ DATA", false },
	{ "CHILD2 WRITE DATA", false },
	{ "PROCESS VALIDATION", false },
};

enum TEST_ENUMS
{
	STEP_DATA_INIT,
	STEP_DATA_RESERVE,
	STEP_CHILD1_LINK_TABLE,
	STEP_CHILD1_LINK_DATA,
	STEP_CHILD1_READ_DATA,
	STEP_CHILD1_WRITE_DATA,
	STEP_CHILD2_LINK_TABLE,
	STEP_CHILD2_LINK_DATA,
	STEP_CHILD2_READ_DATA,
	STEP_CHILD2_WRITE_DATA,
	STEP_PROCESS_VALIDATION,
};

enum TEST_INSTRUCTIONS
{
	CHILD2_LINK_TABLE = 0,
	CHILD2_LINK_DATA = 1,
	CHILD2_READ_DATA = 2,
	TESTPROC1_DONE = 3,
	TESTPROC2_DONE = 4,
	CHILD1_WRITE_DATA = 5,
	CHILD2_WRITE_DATA = 6,
	WRITE = 7,
};

void child_write()
{
	//Write the reverse sequence
	//Allocate the 64 bytes and fill with a sequence (0,1,2...)
	WimpDataArena arena;
	if (wimp_data_access(&arena, "test-sequence") == WIMP_DATA_SUCCESS)
	{
		for (int i = 0; i < 64; ++i)
		{
			WIMP_ARENA_INDEX(arena, i) = 64 - i;
		}

		//Read back sequence
		sds seq_string = sdsempty();
		bool success = true;
		for (int i = 0; i < 64; ++i)
		{
			if (WIMP_ARENA_INDEX(arena, i) != 64 - i)
			{
				success = false;
			}
			seq_string = sdscat(seq_string, sdsfromlonglong((long long)WIMP_ARENA_INDEX(arena, i)));
			seq_string = sdscat(seq_string, " ");
		}
		wimp_log_important("%s\n", seq_string);
		sdsfree(seq_string);
		wimp_data_stop_access(&arena, "test-sequence");

		if (success)
		{
			PASS_MATRIX[STEP_CHILD1_WRITE_DATA].status = true;
		}
	}
}

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
	wimp_init_local_server("test_process1", "127.0.0.1", process_port);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the master process that called this thread
	RecieverArgs args = wimp_get_reciever_args("test_process1", master_domain, master_port, &server->incomingmsg, &server->active);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Add the master process to the table for tracking
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, WIMP_Process_Parent, NULL);

	//Accept the connection to the test_process->master reciever, started by the master thread
	wimp_server_process_accept(server, 1, "master");

	//Link to data table
	if (wimp_data_link_to_process("wimp-master") == WIMP_DATA_SUCCESS)
	{
		PASS_MATRIX[STEP_CHILD1_LINK_TABLE].status = true;
	}

	//Link to the data
	if (wimp_data_link_to_data("test-sequence") == WIMP_DATA_SUCCESS)
	{
		PASS_MATRIX[STEP_CHILD1_LINK_DATA].status = true;
	}

	//Read the data
	WimpDataArena arena;
	if (wimp_data_access(&arena, "test-sequence") == WIMP_DATA_SUCCESS)
	{
		wimp_log_important("Sequence to check:\n");
		sds seq_string = sdsempty();
		bool success = true;
		for (int i = 0; i < 64; ++i)
		{
			if (WIMP_ARENA_INDEX(arena, i) != i)
			{
				success = false;
			}
			seq_string = sdscat(seq_string, sdsfromlonglong((long long)WIMP_ARENA_INDEX(arena, i)));
			seq_string = sdscat(seq_string, " ");
		}
		wimp_log_important("%s\n", seq_string);
		sdsfree(seq_string);
		wimp_data_stop_access(&arena, "test-sequence");

		if (success)
		{
			PASS_MATRIX[STEP_CHILD1_READ_DATA].status = true;
		}
	}

	//Tell the master have read
	wimp_add_local_server("master", TESTPROC1_DONE, NULL, 0);
	wimp_server_send_instructions(server);

	//Loop while waiting for the master to tell to write
	bool disconnect = false;
	while (!disconnect)
	{
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			if (wimp_server_instr_routed(server, meta.dest_process, currentnode))
			{
				//Add to the outgoing and continue to prevent freeing
				currentnode = wimp_instr_queue_pop(&server->incomingmsg);
				continue;
			}

			if (wimp_instr_check(meta.instr, WIMP_INSTRUCTION_EXIT))
			{
				disconnect = true;
			}
			else if (wimp_instr_check(meta.instr, WRITE))
			{
				child_write();
				disconnect = true;
			}

			wimp_instr_node_free(currentnode);
			currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		}
		wimp_instr_queue_high_prio_unlock(&server->incomingmsg);

		wimp_server_send_instructions(server);
	}
	wimp_server_send_instructions(server);

	//Unlink from the data table
	wimp_data_unlink_from_process();

	//Tell the master to close
	wimp_add_local_server("master", WIMP_INSTRUCTION_EXIT, NULL, 0);
	wimp_server_send_instructions(server);

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

	//Init the data table
	if (wimp_data_init("wimp-master") == WIMP_DATA_SUCCESS)
	{
		PASS_MATRIX[STEP_DATA_INIT].status = true;
	}
	else
	{
		wimp_log_fail("Failed to init data!\n");
		return -1;
	}

	//Reserve the data and make the sequence
	if (wimp_data_reserve("test-sequence", 64) == WIMP_DATA_SUCCESS)
	{
		PASS_MATRIX[STEP_DATA_RESERVE].status = true;
	}

	//Allocate the 64 bytes and fill with a sequence (0,1,2...)
	WimpDataArena arena;
	if (wimp_data_access(&arena, "test-sequence") == WIMP_DATA_SUCCESS)
	{
		WArenaPtr ptr = WIMP_ARENA_ALLOC(arena, 64);
		for (int i = 0; i < 64; ++i)
		{
			WIMP_ARENA_INDEX(arena, i) = i;
		}

		//Read back sequence (use uint8_t* to test both
		wimp_log_important("Sequence to check:\n");
		uint8_t* rawptr = WIMP_ARENA_GET_PTR(arena, ptr);
		for (int i = 0; i < 64; ++i)
		{
			wimp_log_important("%d ", (int32_t)rawptr[i]);
		}
		wimp_log("\n");

		wimp_data_stop_access(&arena, "test-sequence");
	}

	//Get unused random ports for the master and end process to run on
	int32_t master_port = wimp_assign_unused_local_port();
	int32_t end_process_port1 = wimp_assign_unused_local_port();
	int32_t end_process_port2 = wimp_assign_unused_local_port();

	//The ports are converted to strings for use as command line arguments
	WimpPortStr port_string1;
	wimp_port_to_string(end_process_port1, port_string1);

	WimpPortStr port_string2;
	wimp_port_to_string(end_process_port2, port_string2);

	WimpPortStr master_port_string;
	wimp_port_to_string(master_port, master_port_string);

	//Start the first client process, creating the command line arguments and creating a new thread
	WimpMainEntry entry1 = wimp_get_entry(4, "--master-port", master_port_string, "--process-port", port_string1);
	wimp_start_library_process("test_process1", (MAIN_FUNC_PTR)&client_main_lib_entry, P_UTHREAD_PRIORITY_LOW, entry1);

	//Start the second client
	WimpMainEntry entry2 = wimp_get_entry(4, "--master-port", master_port_string, "--process-port", port_string2);
	wimp_start_executable_process("test_process2", "WIMP-Test-05_b", entry2);

	//Start a local server for the master process
	wimp_init_local_server("master", "127.0.0.1", master_port);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the client processes that the master started
	RecieverArgs args1 = wimp_get_reciever_args("master", "127.0.0.1", end_process_port1, &server->incomingmsg, &server->active);
	wimp_start_reciever_thread("test_process1", "127.0.0.1", master_port, args1);

	RecieverArgs args2 = wimp_get_reciever_args("master", "127.0.0.1", end_process_port2, &server->incomingmsg, &server->active);
	wimp_start_reciever_thread("test_process2", "127.0.0.1", master_port, args2);

	//Add the test processes to the table for tracking
	wimp_process_table_add(&server->ptable, "test_process1", "127.0.0.1", end_process_port1, WIMP_Process_Child, NULL);
	wimp_process_table_add(&server->ptable, "test_process2", "127.0.0.1", end_process_port2, WIMP_Process_Child, NULL);

	//Accept the connection to the master->test_process reciever, started by the test_process
	wimp_server_process_accept(server, 2, "test_process1", "test_process2");

	//Validate that the process correctly started. Sends a ping packet to make sure is listening
	if (wimp_server_check_process_listening(server, "test_process1")
		&& wimp_server_check_process_listening(server, "test_process2"))
	{
		wimp_log("Process validated!\n");
		PASS_MATRIX[STEP_PROCESS_VALIDATION].status = true;
	}

	p_uthread_sleep(100);

	//This is a simple loop. 
	bool disconnect = false;
	bool tp1_done = false;
	bool tp2_done = false;
	bool tp_sent_instr = false;
	while (!disconnect)
	{
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
			if (wimp_server_instr_routed(server, meta.dest_process, currentnode))
			{
				//Add to the outgoing and continue to prevent freeing
				currentnode = wimp_instr_queue_pop(&server->incomingmsg);
				continue;
			}

			if (wimp_instr_check(meta.instr, WIMP_INSTRUCTION_EXIT))
			{
				disconnect = true;
			}
			else if (wimp_instr_check(meta.instr, WIMP_INSTRUCTION_LOG))
			{
				wimp_log("%s", meta.args);
			}
			else if (wimp_instr_check(meta.instr, CHILD2_LINK_TABLE))
			{
				PASS_MATRIX[STEP_CHILD2_LINK_TABLE].status = true;
			}
			else if (wimp_instr_check(meta.instr, CHILD2_LINK_DATA))
			{
				PASS_MATRIX[STEP_CHILD2_LINK_DATA].status = true;
			}
			else if (wimp_instr_check(meta.instr, CHILD2_READ_DATA))
			{
				PASS_MATRIX[STEP_CHILD2_READ_DATA].status = true;
			}
			else if (wimp_instr_check(meta.instr, TESTPROC1_DONE))
			{
				wimp_log_important("test process 1 done\n");
				tp1_done = true;
			}
			else if (wimp_instr_check(meta.instr, TESTPROC2_DONE))
			{
				wimp_log_important("test process 2 done\n");
				tp2_done = true;
			}
			else if (wimp_instr_check(meta.instr, CHILD1_WRITE_DATA))
			{
				PASS_MATRIX[STEP_CHILD1_WRITE_DATA].status = true;
			}
			else if (wimp_instr_check(meta.instr, CHILD2_WRITE_DATA))
			{
				PASS_MATRIX[STEP_CHILD2_WRITE_DATA].status = true;
			}

			wimp_instr_node_free(currentnode);
			currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		}
		wimp_instr_queue_high_prio_unlock(&server->incomingmsg);

		if (tp1_done && tp2_done && !tp_sent_instr)
		{
			wimp_log_important("Both done, next step...\n");
			wimp_add_local_server("test_process1", WRITE, NULL, 0);
			wimp_add_local_server("test_process2", WRITE, NULL, 0);
			wimp_server_send_instructions(server);
			tp_sent_instr = true;
		}
		wimp_server_send_instructions(server);
	}

	//Cleanup
	wimp_log("Master thread closed\n");
	wimp_close_local_server();

	//Cleanup
	wimp_data_free();
	wimp_shutdown();

	wimp_test_validate_passmat(PASS_MATRIX, 11);
	return 0;
}