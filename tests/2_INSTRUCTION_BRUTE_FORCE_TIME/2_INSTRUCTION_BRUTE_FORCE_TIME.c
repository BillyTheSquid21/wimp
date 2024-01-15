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
	{ "SHORT INSTRUCTIONS ARRIVED", false},
	{ "LONG INSTRUCTIONS ARRIVED", false},
	{ "SHORT BATCH ARRIVED", false},
	{ "LONG BATCH ARRIVED", false}
};

enum TEST_ENUMS
{
	STEP_PROCESS_VALIDATION,
	SHORT_INSTRUCTIONS_ARRIVED,
	LONG_INSTRUCTIONS_ARRIVED,
	SHORT_BATCH_ARRIVED,
	LONG_BATCH_ARRIVED
};

#define SHORT_INSTR_COUNT 500000
#define LONG_INSTR_COUNT 100000

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
	wimp_init_local_server("test_process", "127.0.0.1", process_port, WIMP_SERVERTYPE_MASTER);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the master process that called this thread
	RecieverArgs args = wimp_get_reciever_args("test_process", master_domain, master_port, &server->incomingmsg);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Add the master process to the table for tracking
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, NULL);

	//Accept the connection to the test_process->master reciever, started by the master thread
	wimp_server_process_accept(server, 1, "master");

	//This server won't loop, so send some instructions to the master thread
	
	//Instruction 1 - This sends the short instruction - in one go
	timer_start(&PASS_MATRIX[SHORT_INSTRUCTIONS_ARRIVED].timer);
	wimp_log("Sending %d short instructions...\n", SHORT_INSTR_COUNT);
	for (int32_t i = 0; i < SHORT_INSTR_COUNT; ++i)
	{
		wimp_add_local_server("master", "incr_sht", NULL, 0);
		if (i % (SHORT_INSTR_COUNT / 10) == 0)
		{
			float pc = (float)i / (float)SHORT_INSTR_COUNT;
			pc *= 100.0f;

			wimp_log("%f pc added!\n", pc);
		}
	}
	wimp_log("Done!\n");

	//This tells the server to send off the instructions
	wimp_log("Sending...\n");
	wimp_server_send_instructions(server);
	wimp_log("Done!\n");

	//Instruction 2 - This sends the long instruction - in one go
	timer_start(&PASS_MATRIX[LONG_INSTRUCTIONS_ARRIVED].timer);
	int32_t long_instr_args[64];
	memset(long_instr_args, 64, 64);

	wimp_log("Sending %d long instructions...\n", LONG_INSTR_COUNT);
	for (int32_t i = 0; i < LONG_INSTR_COUNT; ++i)
	{
		wimp_add_local_server("master", "increment_counter_long", &long_instr_args[0], 64 * sizeof(int32_t));
		if (i % (LONG_INSTR_COUNT / 10) == 0)
		{
			float pc = (float)i / (float)LONG_INSTR_COUNT;
			pc *= 100.0f;

			wimp_log("%f pc added!\n", pc);
		}
	}
	wimp_log("Done!\n");

	//This tells the server to send off the instructions
	wimp_log("Sending...\n");
	wimp_server_send_instructions(server);
	wimp_log("Done!\n");

	//Instruction 3 - This sends the short instruction - in batches of 1000 of the total
	timer_start(&PASS_MATRIX[SHORT_BATCH_ARRIVED].timer);
	wimp_log("Sending %d short instructions in batch...\n", SHORT_INSTR_COUNT);
	for (int32_t i = 0; i < SHORT_INSTR_COUNT; ++i)
	{
		wimp_add_local_server("master", "shrt_btc", NULL, 0);
		if (i % (SHORT_INSTR_COUNT / 10) == 0)
		{
			float pc = (float)i / (float)SHORT_INSTR_COUNT;
			pc *= 100.0f;

			wimp_log("%f pc added!\n", pc);
		}
		if (i % 1000 == 0)
		{
			wimp_server_send_instructions(server);
		}
	}
	wimp_log("Done!\n");

	wimp_log("Sending...\n");
	wimp_server_send_instructions(server); //just in case
	wimp_log("Done!\n");

	//Instruction 4 - This sends the long instruction - in batches of 1000 of the total
	timer_start(&PASS_MATRIX[LONG_BATCH_ARRIVED].timer);
	wimp_log("Sending %d long instructions in batch...\n", LONG_INSTR_COUNT);
	for (int32_t i = 0; i < LONG_INSTR_COUNT; ++i)
	{
		wimp_add_local_server("master", "batched64_counter_long", NULL, 0);
		if (i % (LONG_INSTR_COUNT / 10) == 0)
		{
			float pc = (float)i / (float)LONG_INSTR_COUNT;
			pc *= 100.0f;

			wimp_log("%f pc added!\n", pc);
		}
		if (i % 1000 == 0)
		{
			wimp_server_send_instructions(server);
		}
	}
	wimp_log("Done!\n");

	wimp_log("Sending...\n");
	wimp_server_send_instructions(server); //just in case
	wimp_log("Done!\n");

	//Exit
	wimp_add_local_server("master", "exit", NULL, 0);
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
	p_libsys_init();

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
	wimp_start_library_process("test_process", (MAIN_FUNC_PTR)&client_main_lib_entry, entry);

	//Start a local server for the master process
	wimp_init_local_server("master", "127.0.0.1", master_port, WIMP_SERVERTYPE_MASTER);
	WimpServer* server = wimp_get_local_server();

	//Start a reciever thread for the client process that the master started
	RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", end_process_port, &server->incomingmsg);
	wimp_start_reciever_thread("test_process", "127.0.0.1", master_port, args);

	//Add the test process to the table for tracking
	wimp_process_table_add(&server->ptable, "test_process", "127.0.0.1", end_process_port, NULL);

	//Accept the connection to the master->test_process reciever, started by the test_process
	wimp_server_process_accept(server, 1, "test_process");

	//Validate that the process correctly started. Sends a ping packet to make sure is listening
	if (wimp_server_validate_process(server, "test_process"))
	{
		wimp_log("Process validated!\n");
		PASS_MATRIX[STEP_PROCESS_VALIDATION].status = true;
	}

	//This is a simple loop. 
	bool disconnect = false;
	int32_t inc_sht_counter = 0;
	int32_t inc_lng_counter = 0;
	int32_t inc_sht_btch_counter = 0;
	int32_t inc_lng_btch_counter = 0;
	while (!disconnect)
	{
		wimp_instr_queue_high_prio_lock(&server->incomingmsg);
		WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
		while (currentnode != NULL)
		{
			WimpInstrMeta meta = wimp_get_instr_from_buffer(currentnode->instr.instruction, currentnode->instr.instruction_bytes);
			if (strcmp(meta.instr, "incr_sht") == 0)
			{
				if (inc_sht_counter % (SHORT_INSTR_COUNT / 10) == 0)
				{
					float pc = (float)inc_sht_counter / (float)SHORT_INSTR_COUNT;
					pc *= 100.0f;

					wimp_log("%f pc short recieved!\n", pc);
				}
				inc_sht_counter++;

				if (inc_sht_counter == SHORT_INSTR_COUNT)
				{
					timer_end(&PASS_MATRIX[SHORT_INSTRUCTIONS_ARRIVED].timer);
				}
			}
			else if (strcmp(meta.instr, "increment_counter_long") == 0)
			{
				if (inc_lng_counter % (LONG_INSTR_COUNT / 10) == 0)
				{
					float pc = (float)inc_lng_counter / (float)LONG_INSTR_COUNT;
					pc *= 100.0f;

					wimp_log("%f pc long recieved!\n", pc);
				}
				inc_lng_counter++;

				if (inc_lng_counter == LONG_INSTR_COUNT)
				{
					timer_end(&PASS_MATRIX[LONG_INSTRUCTIONS_ARRIVED].timer);
				}
			}
			else if (strcmp(meta.instr, "shrt_btc") == 0)
			{
				if (inc_sht_btch_counter % (SHORT_INSTR_COUNT / 10) == 0)
				{
					float pc = (float)inc_sht_btch_counter / (float)SHORT_INSTR_COUNT;
					pc *= 100.0f;

					wimp_log("%f pc short batch recieved!\n", pc);
				}
				inc_sht_btch_counter++;

				if (inc_sht_btch_counter == SHORT_INSTR_COUNT)
				{
					timer_end(&PASS_MATRIX[SHORT_BATCH_ARRIVED].timer);
				}
			}
			else if (strcmp(meta.instr, "batched64_counter_long") == 0)
			{
				if (inc_lng_btch_counter % (LONG_INSTR_COUNT / 10) == 0)
				{
					float pc = (float)inc_lng_btch_counter / (float)LONG_INSTR_COUNT;
					pc *= 100.0f;

					wimp_log("%f pc long batch recieved!\n", pc);
				}
				inc_lng_btch_counter++;

				if (inc_lng_btch_counter == LONG_INSTR_COUNT)
				{
					timer_end(&PASS_MATRIX[LONG_BATCH_ARRIVED].timer);
				}
			}
			else if (strcmp(meta.instr, WIMP_INSTRUCTION_EXIT) == 0)
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

	//Validate all instructions arrived
	if (inc_sht_counter / SHORT_INSTR_COUNT == 1)
	{
		PASS_MATRIX[SHORT_INSTRUCTIONS_ARRIVED].status = true;
	}

	if (inc_lng_counter / LONG_INSTR_COUNT == 1)
	{
		PASS_MATRIX[LONG_INSTRUCTIONS_ARRIVED].status = true;
	}

	if (inc_sht_btch_counter / SHORT_INSTR_COUNT == 1)
	{
		PASS_MATRIX[SHORT_BATCH_ARRIVED].status = true;
	}

	if (inc_lng_btch_counter / LONG_INSTR_COUNT == 1)
	{
		PASS_MATRIX[LONG_BATCH_ARRIVED].status = true;
	}

	//Print benchmark status as this is not a default thing
	float s1time = get_time_elapsed(PASS_MATRIX[SHORT_INSTRUCTIONS_ARRIVED].timer);
	wimp_log("\nStep 1 time taken: %f s\n", s1time);
	wimp_log("\nStep 1 time per instr: %f ms\n", (s1time / (float)SHORT_INSTR_COUNT) * 1000.0f);
	wimp_log("\nStep 1 instr per second: %f\n", (float)SHORT_INSTR_COUNT / s1time);

	float s2time = get_time_elapsed(PASS_MATRIX[LONG_INSTRUCTIONS_ARRIVED].timer);
	wimp_log("\nStep 2 time taken: %f s\n", s2time);
	wimp_log("\nStep 2 time per instr: %f ms\n", (s2time / (float)LONG_INSTR_COUNT) * 1000.0f);
	wimp_log("\nStep 2 instr per second: %f\n", (float)LONG_INSTR_COUNT / s2time);

	float s3time = get_time_elapsed(PASS_MATRIX[SHORT_BATCH_ARRIVED].timer);
	wimp_log("\nStep 3 time taken: %f s\n", s3time);
	wimp_log("\nStep 3 time per instr: %f ms\n", (s3time / (float)SHORT_INSTR_COUNT) * 1000.0f);
	wimp_log("\nStep 3 instr per second: %f\n", (float)SHORT_INSTR_COUNT / s3time);

	float s4time = get_time_elapsed(PASS_MATRIX[LONG_BATCH_ARRIVED].timer);
	wimp_log("\nStep 4 time taken: %f s\n", s4time);
	wimp_log("\nStep 4 time per instr: %f ms\n", (s4time / (float)LONG_INSTR_COUNT) * 1000.0f);
	wimp_log("\nStep 4 instr per second: %f\n", (float)LONG_INSTR_COUNT / s4time);

	//Cleanup
	wimp_log("Master thread closed\n");
	wimp_close_local_server();

	//Cleanup
	p_libsys_shutdown();

	wimp_test_validate_passmat(PASS_MATRIX, 5);
	return 0;
}