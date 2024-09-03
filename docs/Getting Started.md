### CMake

To use WIMP in a CMake project, ensure the repository has been cloned recursively, as otherwise the dependencies will not be included. This should be to a place in your own CMake project. Then all you need to do is use `add_subdirectory(wimp)` in your CMakeLists.txt file. And that's all that's needed to include the project!

### Library Initialization

To use the library simply include the "wimp.h" header. Initializing the library is very simple, and is only one call:
[wimp_init()](https://docs.hdoc.io/billythesquid/WIMP/fBCB46A4E40C621C4.html)

To shutdown, simply call a corresponding:
[wimp_shutdown()](https://docs.hdoc.io/billythesquid/WIMP/fAD292975D80D2D11.html)

These calls should be called at the start and end of a process respectively. Reference counting is used to ensure that the library stays active for the duration of a program.

### Create a server for a process

A server only needs two things, an address, and a port. The port *must* be unique to the process. A function therefore is provided to assign a local random unused port, although any port can be used if desired. A server can be directly created, however the preferred method is to create a local server. This creates a thread local server that can be accessed from anywhere in the thread without having to pass a struct pointer around, which can also be a benefit when calling from non C/C++ code. A local server state could be set up by calling [wimp_init_local_server()](https://docs.hdoc.io/billythesquid/WIMP/f52B924C25076A05B.html) like this:

````
int32_t master_port = wimp_assign_unused_local_port();
wimp_init_local_server("master", "127.0.0.1", master_port);
WimpServer* server = wimp_get_local_server();
````

to cleanup the local server, call [wimp_close_local_server()](https://docs.hdoc.io/billythesquid/WIMP/f7AAFE5F6C6C04A8A.html)

### Launch a process

A process can either be launched from a C/C++ function pointer, or from an executable in the binary directory. Both the parent and the child process need to know each other's address and port, so unless the port is known at compile time, the process needs to be launched with the addresses and ports provided as arguments. This is done by creating a ```WimpMainEntry``` struct instance. An example of both a binary launch and a function pointer launch would look like this:

````
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
wimp_start_executable_process("test_process2", "executable.exe", entry2);
````

Both the binary and the functions ran as processes should have the standard C entry pointer arguements, e.g. ```int client_main_entry(int argc, char** argv)```

### Setup the reciever and connect to the server

For each other process a process wants to recieve from, whether thats a master or child, you want to create a reciever thread, add the process to recieve from to the server process table, and accept the connection:

```
//Start a reciever thread for the client process that the master started
RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", end_process_port, &server->incomingmsg);
wimp_start_reciever_thread("test_process", "127.0.0.1", master_port, args);

//Add the test process to the table for tracking
wimp_process_table_add(&server->ptable, "test_process", "127.0.0.1", end_process_port, WIMP_Process_Child, NULL);

//Accept the connection to the master->test_process reciever, started by the test_process
wimp_server_process_accept(server, 1, "test_process");
```

### Recieving/Sending instructions

By this point the servers are connected, but aren't sending anything. A while loop can easily be set up to recieve instructions - this isn't strictly necessary but the user needs to consider how their program will actually run. Instructions are recieved as nodes in an instruction queue, from which the string based metadata can be read and used. An example loop would look like this:

```
//This is a simple loop. 
bool disconnect = false;
while (!disconnect)
{
	wimp_instr_queue_high_prio_lock(&server->incomingmsg);
	WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
	while (currentnode != NULL)
	{
		WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
		if (wimp_instr_check(meta.instr, "blank_instr") == 0)
		{
			wimp_log("\n");
		}
		else if (wimp_instr_check(meta.instr, "say_hello") == 0)
		{
			wimp_log("HELLO!\n");
		}
		else if (wimp_instr_check(meta.instr, "echo") == 0)
		{
			//Get the arguments
			const char* echo_string = (const char*)meta.args;
			wimp_log("%s\n", echo_string);
		}
		else if (wimp_instr_check(meta.instr, WIMP_INSTRUCTION_EXIT) == 0)
		{
			wimp_log("\n");
			disconnect = true;
		}

		wimp_instr_node_free(currentnode);
		currentnode = wimp_instr_queue_pop(&server->incomingmsg);
	}
	wimp_instr_queue_high_prio_unlock(&server->incomingmsg);
}
```
When recieving instructions the queue should be locked with the high prio flag - this is to allow all instructions in the queue to be read quickly without the reciever writing instructions. Then an inner loop is formed by checking if the node is null, while processing each node and popping to get the next.

### Shared Data

A shared data space can be created by calling [wimp_data_init("data_space_name")](https://docs.hdoc.io/billythesquid/WIMP/f15D3A968992199E7.html) and destroyed by calling [wimp_data_free()](https://docs.hdoc.io/billythesquid/WIMP/f58A37E54B16836C8.html). This *must* only be called once per program, in a master process that lasts the duration that data may be accessed. Any other process must instead link to the initialized data by calling [wimp_data_link_to_process("data_space_name")](https://docs.hdoc.io/billythesquid/WIMP/f0B2CCC120AD23D1F.html) and unlinked by calling [wimp_data_unlink_from_process()](https://docs.hdoc.io/billythesquid/WIMP/fA3E5D92036D59575.html). Each data space has a specific amount of slots defined by the macro ```WIMP_MAX_SHARED_SLOTS``` (normally 64).

Reserving data is simple, and can be done by calling [wimp_data_reserve("data_name", data_size)](https://docs.hdoc.io/billythesquid/WIMP/f87DFDE8016ED5DB7.html). It is accessed through an arena structure, an example of access would be like this:

```
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
```

A WArenaPtr is simply a ```size_t``` type that is an offset into the arena, regardless of what raw pointer address the memory has been mapped to. Arena access can be done using the WIMP_ARENA macros.
