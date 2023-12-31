#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp_reciever.h>
#include <wimp_process.h>
#include <wimp_process_table.h>
#include <wimp_server.h>
#include <wimp_instruction.h>

int client_main_entry(int argc, char** argv)
{
	printf("Test process!\n");

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

	wimp_init_local_server("test_process", "127.0.0.1", process_port);
	WimpServer* server = wimp_get_local_server();

	RecieverArgs args = wimp_get_reciever_args("test_process", master_domain, master_port, &server->incomingmsg);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Process table
	wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, NULL);

	//Connect here - will in future call to accept connection immediately after
	//starting process
	wimp_server_process_accept(server, "master");

	wimp_send_local_server("master", "test", NULL, 0);
	wimp_send_local_server("master", "test2", NULL, 0);
	wimp_server_send_instructions(server);

	p_uthread_sleep(6000);

	// Cleanup
	printf("Server thread closed: %d\n", server);
	wimp_close_local_server();

	return 0;
}

int client_main_lib_entry(WimpMainEntry entry)
{
	int res = client_main_entry(entry->argc, entry->argv);
	wimp_free_entry(entry);
	printf("Finishing test process!\n");
	return res;
}

int main(void) 
{
	p_libsys_init();

	// Start child process and launches a reciever that recieves from the end process
	int32_t writebuffer = 0;

	//Need a port for the master the end reciever reads from
	//And a port for the end process the master reciever reads from
	int32_t master_port = wimp_assign_unused_local_port();
	int32_t end_process_port = wimp_assign_unused_local_port();

	WimpPortStr port_string;
	wimp_port_to_string(end_process_port, &port_string);

	WimpPortStr master_port_string;
	wimp_port_to_string(master_port, &master_port_string);

	WimpMainEntry entry = wimp_get_entry(4, "--master-port", master_port_string, "--process-port", port_string);
	wimp_start_library_process("test_process", &client_main_lib_entry, entry);

	wimp_init_local_server("master", "127.0.0.1", master_port);
	WimpServer* server = wimp_get_local_server();

	//Process table
	WimpInstrQueue* queue = &server->incomingmsg;
	RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", end_process_port, &server->incomingmsg);

	wimp_start_reciever_thread("test_process", "127.0.0.1", master_port, args);

	wimp_process_table_add(&server->ptable, "test_process", "127.0.0.1", end_process_port, NULL);

	//Connect here - will in future call to accept connection immediately after
	//starting process
	wimp_server_process_accept(server, "test_process");

	if (wimp_server_validate_process(server, "test_process"))
	{
		printf("Process Validated!\n");
	}

	p_uthread_sleep(6000);

	//Cleanup
	printf("Server thread closed: %d\n", server);
	wimp_close_local_server();

	//Cleanup
	p_libsys_shutdown();
	return 0;
}