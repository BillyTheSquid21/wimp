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
			wimp_add_local_server("master", "STEP_CHILD2_WRITE_DATA", NULL, 0);
		}
	}
}

/*
* This is an example client main. It takes the domains and ports as cmd arguments and creates and starts a server.
* After sending the commands, the server closes.
*/
int main(int argc, char** argv)
{
	//Initialize the socket library
	wimp_init();

	//Default this domain and port
	const char* process_domain = "127.0.0.1";
	int32_t process_port = 8001;

	//Default the master domain and port
	const char* master_domain = "127.0.0.1";
	int32_t master_port = 8000;

	//Read the args, look for the --master and --proc args
	for (int i = 0; i < argc; ++i)
	{
		printf("arg[%d] %s\n", i, argv[i]);
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
	wimp_init_local_server("test_process2", "127.0.0.1", process_port);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the master process that called this thread
	RecieverArgs args = wimp_get_reciever_args("test_process2", master_domain, master_port, &server->incomingmsg, &server->active);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Add the master process to the table for tracking
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, WIMP_Process_Parent, NULL);

	//Accept the connection to the test_process->master reciever, started by the master thread
	wimp_server_process_accept(server, 1, "master");

	//Link to data table
	if (wimp_data_link_to_process("wimp-master") == WIMP_DATA_SUCCESS)
	{
		wimp_add_local_server("master", "STEP_CHILD2_LINK_TABLE", NULL, 0);
	}

	//Link to the data
	if (wimp_data_link_to_data("test-sequence") == WIMP_DATA_SUCCESS)
	{
		wimp_add_local_server("master", "STEP_CHILD2_LINK_DATA", NULL, 0);
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
			wimp_add_local_server("master", "STEP_CHILD2_READ_DATA", NULL, 0);
		}
	}

	//Tell the master have read
	wimp_add_local_server("master", "testproc2_done", NULL, 0);
	wimp_server_send_instructions(server);

	//Loop while waiting for the master to tell to write
	//As is a separate executable, check if the parent is alive.
	bool disconnect = false;
	while (!disconnect && wimp_server_is_parent_alive(server))
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

			if (strcmp(meta.instr, WIMP_INSTRUCTION_EXIT) == 0)
			{
				disconnect = true;
			}
			else if (strcmp(meta.instr, "write") == 0)
			{
				wimp_add_local_server("master", "STEP_CHILD2_WRITE_DATA", NULL, 0);
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
	wimp_close_local_server();

	//Cleanup
	wimp_shutdown();

	return 0;
}