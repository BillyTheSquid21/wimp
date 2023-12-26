#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wimp_reciever.h>
#include <wimp_process.h>
#include <wimp_process_table.h>

#define MAX_MESSAGE_LENGTH 1024
#define SERVER_PORT 5432
#define END_PROCESS_PORT 5433

int client_main_entry(int argc, char** argv)
{
	printf("Test process!\n");
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

	//Need a port for the master to send to the end reciever
	//And a port for the end process to send to the master reciever
	int32_t master_port = wimp_assign_unused_local_port();
	int32_t end_process_port = wimp_assign_unused_local_port();

	wimp_port_str port_string;
	wimp_port_to_string(end_process_port, &port_string);

	wimp_port_str master_port_string;
	wimp_port_to_string(master_port, &master_port_string);

	WimpMainEntry entry = wimp_get_entry(2, "Hello", "World");
	
	wimp_start_library_process("test_process", &client_main_lib_entry, entry);

	RecieverArgs args = wimp_get_reciever_args("dom1", 1, "dom2", 2, NULL);
	wimp_start_reciever_thread("rec1", args);

	p_uthread_sleep(1000);

	// Cleanup
	p_libsys_shutdown();
	return 0;
}