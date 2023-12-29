#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp_reciever.h>
#include <wimp_process.h>
#include <wimp_process_table.h>

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

	//Process table
	WimpProcessTable ptable = wimp_create_process_table();
	wimp_process_table_add(&ptable, "master", "127.0.0.1", master_port, NULL);

	RecieverArgs args = wimp_get_reciever_args("test_process", master_domain, master_port, NULL);
	wimp_start_reciever_thread("master", process_domain, process_port, args);

	//Create the server socket then create a connection to the test_proc reciever
	//the master process has created
	//Create the server socket, then create a connection to the master reciever
	//the end process has created
	WimpMsgBuffer recbuffer;
	WimpMsgBuffer sendbuffer;
	WIMP_ZERO_BUFFER(recbuffer); WIMP_ZERO_BUFFER(sendbuffer);

	PSocketAddress* addr;
	PSocket* server;

	if ((addr = p_socket_address_new("127.0.0.1", process_port)) == NULL)
	{
		return 1;
	}

	if ((server = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		// Failed to create socket -- cleanup
		p_socket_address_free(addr);

		return 2;
	}

	if (!p_socket_bind(server, addr, FALSE, NULL))
	{
		// Couldn't bind socket, cleanup
		p_socket_address_free(addr);
		p_socket_free(server);

		return 3;
	}

	if (!p_socket_listen(server, NULL))
	{
		// Couldn't start listening, cleanup
		p_socket_address_free(addr);
		p_socket_close(server, NULL);

		return 4;
	}

	//Connect here - will in future call to accept connection immediately after
	//starting process
	wimp_process_accept(&ptable, server, recbuffer, sendbuffer);

	// Cleanup
	printf("Server thread closed: %d\n", server);
	p_socket_address_free(addr);
	p_socket_close(server, NULL);

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
	
	//Process table
	WimpProcessTable ptable = wimp_create_process_table();
	wimp_process_table_add(&ptable, "test_process", "127.0.0.1", end_process_port, NULL);

	RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", end_process_port, "127.0.0.1", master_port, NULL);
	wimp_start_reciever_thread("test_process", "127.0.0.1", master_port, args);

	//Create the server socket, then create a connection to the master reciever
	//the end process has created
	WimpMsgBuffer recbuffer;
	WimpMsgBuffer sendbuffer;
	WIMP_ZERO_BUFFER(recbuffer); WIMP_ZERO_BUFFER(sendbuffer);

	PSocketAddress* addr;
	PSocket* server;

	if ((addr = p_socket_address_new("127.0.0.1", master_port)) == NULL)
	{
		return 1;
	}

	if ((server = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		// Failed to create socket -- cleanup
		p_socket_address_free(addr);

		return 2;
	}

	if (!p_socket_bind(server, addr, FALSE, NULL))
	{
		// Couldn't bind socket, cleanup
		p_socket_address_free(addr);
		p_socket_free(server);

		return 3;
	}

	if (!p_socket_listen(server, NULL))
	{
		// Couldn't start listening, cleanup
		p_socket_address_free(addr);
		p_socket_close(server, NULL);

		return 4;
	}

	//Connect here - will in future call to accept connection immediately after
	//starting process
	wimp_process_accept(&ptable, server, recbuffer, sendbuffer);

	// Cleanup
	printf("Server thread closed: %d\n", server);
	p_socket_address_free(addr);
	p_socket_close(server, NULL);

	p_uthread_sleep(6000);

	// Cleanup
	p_libsys_shutdown();
	return 0;
}