#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp_reciever.h>
#include <wimp_process.h>
#include <wimp_process_table.h>
#include <wimp_server.h>
#include <wimp_instruction.h>
#include <wimp_test.h>

/*
* This is an example client main. It takes the domains and ports as cmd arguments and creates and starts a server.
* After sending the commands, the server closes.
*/
int main(int argc, char** argv)
{
	//Initialize the socket library
	wimp_init();

	wimp_log("Test process with arg count: %d\n", argc);

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

	//This server won't loop, so send some instructions to the master thread
	//Ensure the log message shows up on the master terminal
	wimp_log("Sending instructions to master!\n");
	
	//Instruction 1 - This sends a simple instr that the master will ignore. It has no additional arguments
	wimp_add_local_server("master", "blank_instr", NULL, 0);

	//Instruction 2 - This sends a simple instr that tells the master to say hello. It has no additional arguments
	wimp_add_local_server("master", "say_hello", NULL, 0);

	//Instruction 3 - This sends a more complex instr, that tells the master to echo the string sent.
	const char* echo_string = "Echo!";
	wimp_add_local_server("master", "echo", echo_string, (strlen(echo_string) + 1) * sizeof(char));

	//Instruction 4 - This simple tells the master to exit - TODO: This implementation may change in the future
	wimp_add_local_server("master", "exit", NULL, 0);

	wimp_log("Instructions sent to master!\n");

	//This tells the server to send off the instructions
	wimp_server_send_instructions(server);

	//This should also shut down the reciever
	p_uthread_sleep(100);
	wimp_close_local_server();

	//Cleanup
	wimp_shutdown();

	return 0;
}