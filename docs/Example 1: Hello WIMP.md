
### Hello WIMP

Creating a WIMP system is pretty simple, but first you'll want to ensure everything is setup correctly. This tutorial is for using WIMP in C/C++ and so will use CMake - this is currently the only supported build automation tool, although I'm sure an enterprising individual can use the project with alternative systems such as MAKE. This tutorial covers a lot of boilerplate, so bear with it, later tutorials (shouldn't) be as long!

### CMake

To use WIMP in a CMake project, ensure the repository has been cloned recursively, as otherwise the dependencies will not be included. This should be to a place in your own CMake project. Then all you need to do is use `add_subdirectory(wimp)` in your CMakeLists.txt file. And that's all that's needed to include the project!

### The server

In WIMP, the server is the core of all communication, so I'll go into explaining it a bit. You can think of it as being the interface through which a process talks to other processes. An example usage would be a simple game with three processes: a render process for drawing objects to the screen, a game process for managing gameplay and a main process which handles windowing and input, and starts the other two (this is just an example and may not be the optimal way to do any of those things but I digress!). The 3 processes can operate independently of each other, each on their own thread - the game itself doesn't need to wait for input to poll, the renderer doesn't need to wait for objects to move to draw and the main thread doesn't need to know the result of the gameplay. They do however, need to coordinate. The game needs to tell the renderer to draw an object in a different place, the main process needs to tell the game that the player has clicked the mouse and the renderer really doesn't need to tell the other two processes anything. But if they need to communicate sometimes, but otherwise can run on their own threads independently, how can we communicate between them? Well, this is where the server comes in.

A server can send instructions to another server, and forget about them. This means you could, for example, send an instruction from the game process to the render process ("draw this sprite at xyz position") and continue performing game calculations. The game doesn't need to wait for the sprite to draw, it just tells the renderer to do it and carries on with it's day. This is great as it allows us to fully decouple systems from each other, and makes interprocess communication (IPC) easier. So let's start writing some code to use the server.

### Initializing WIMP

The code I write will be in C, and so will be valid C++ code as well, however some C practices are considered bad C++ practices so bear that in mind!

The first thing you need to do, is include the `"wimp.h"` header. This provides all the functionality of the library, and wraps things conveniently in an extern "C" block for C++ users. Then we call [wimp_init()](https://docs.hdoc.io/billythesquid/WIMP/fBCB46A4E40C621C4.html)  and follow up with [wimp_shutdown()](https://docs.hdoc.io/billythesquid/WIMP/fAD292975D80D2D11.html). We should also make a habit of checking the result of our operations! We don't want to use WIMP if for whatever reason it doesn't start. So the starting code is this:
    
    #include <wimp.h>
    
    int main(void)
    {
    	if (wimp_init() != WIMP_PROCESS_SUCCESS)
    	{
    		printf("WIMP failed :(");
    		return -1;
    	}
    
    	printf("WIMP succeeded :)");
    	wimp_shutdown();
    	return 0;
    }
This should hopefully output that WIMP succeeded. This code should be called in every process and should last for it's lifetime. But it doesn't do anything right now, so lets change that.

### Creating your first server

Creating a server is very simple. It helps to think of the server at this moment as being a networking socket that (usually) runs on the local computer (this is why the option to specify domain is presented although currently WIMP only supports local servers). So all that makes a server unique is it's domain, it's name and it's port. The domain should be localhost, which is "127.0.0.1" in IPv4 world, and the library offers a handy function to find a local unused port on your system to run the server on. So creating a local server should be done like this:

    int main(void)
    {
    	if (wimp_init() != WIMP_PROCESS_SUCCESS)
    	{
    		printf("WIMP failed :(");
    		return -1;
    	}
    
    	//Get the master port and init local server
    	int32_t master_port = wimp_assign_unused_local_port();
    	if (wimp_init_local_server("master", "127.0.0.1", master_port) != WIMP_SERVER_SUCCESS)
    	{
    		printf("Failed to init the local server!!!\n");
    		wimp_shutdown();
    		return -2;
    	}
    	printf("WIMP succeeded on port: %d\n", master_port);
    
    	wimp_close_local_server();
    	wimp_shutdown();
    	return 0;
    }
This will allocate the local server an unused port and start it with [wimp_init_local_server()](https://docs.hdoc.io/billythesquid/WIMP/f52B924C25076A05B.html). Again we check the result to make sure nothing has gone catastrophically wrong. The result should be we print the port the server briefly ran on before shutting down due to a call to [wimp_close_local_server()](https://docs.hdoc.io/billythesquid/WIMP/f7AAFE5F6C6C04A8A.html). All it means to create a local server is that behind the scenes there is a global server variable that is thread local and we can access without having to pass a pointer around every function call. Very helpful. So how do we do something useful with this?

### Connecting servers

To connect our server to something, we first need another thread with it's own server to connect to. The WIMP library offers helper functions for spooling up a new thread with the information it needs to connect. I'll show the code first then explain what each new part is doing as a lot is going on.

    int client_main(int argc, char** argv)
    {
    	printf("I started!\n");
    	return 0;
    }
    
    int client_main_entry(WimpMainEntry entry)
    {
    	int res = client_main(entry->argc, entry->argv);
    	wimp_free_entry(entry);
    	return res;
    }
    
    int main(void)
    {
    	if (wimp_init() != WIMP_PROCESS_SUCCESS)
    	{
    		printf("WIMP failed :(");
    		return -1;
    	}
    
    	//Get the master port and init local server
    	int32_t master_port = wimp_assign_unused_local_port();
    	if (wimp_init_local_server("master", "127.0.0.1", master_port) != WIMP_SERVER_SUCCESS)
    	{
    		printf("Failed to init the local server!!!\n");
    		wimp_shutdown();
    		return -2;
    	}
    	printf("WIMP succeeded on port: %d\n", master_port);
    
    	//Get the client port and convert both ports to strings
    	int32_t process_port = wimp_assign_unused_local_port();
    	WimpPortStr port_string;
    	WimpPortStr master_port_string;
    	wimp_port_to_string(process_port, port_string);
    	wimp_port_to_string(master_port, master_port_string);
    
    	//Start the client process
    	WimpMainEntry entry = wimp_get_entry(4, "--master-port", master_port_string, "--process-port", port_string);
    	if (wimp_start_library_process("client", (MAIN_FUNC_PTR)&client_main_entry, P_UTHREAD_PRIORITY_NORMAL, entry) != WIMP_PROCESS_SUCCESS)
    	{
    		printf("Failed to start library process!\n");
    		wimp_close_local_server();
    		wimp_shutdown();
    		return -3;
    	}
    
    	wimp_close_local_server();
    	wimp_shutdown();
    	return 0;
    }
So first you'll notice we now have two new functions defined, **client_main** and **client_main_entry** - these are the functions that will run on our client server. The main has typical int `argc` and `argv` parameters a C style main function would have, and the entry is ran with a **WimpMainEntry** input - this essentially just packs up the C style arguments (which has an array of strings in `argv`) so they can easily be passed around and freed. Why is it done this way? Well, it allows the same main function to be used for running as a standalone executable with command line arguments, or as a function linked to the library launched on a new thread, and both act the same. As we are automatically assigning our local ports, the client doesn't automatically know the port the master is running on and so passing this as a command line string style parameter lets it know where to look for it's parent. If the ports and domains are fixed at compile time, this can be omitted and entry replaced with `NULL`. We also convert the ports to stack strings for this as command line arguments don't place nicely with raw numerical values. We then create these entry arguments with [wimp_get_entry(4, "--master-port", master_port_string, "--process-port", port_string)](https://docs.hdoc.io/billythesquid/WIMP/fF7409261BE42D7FE.html), making sure to specify the number of arguments at the start - any arguments can be sent this way as the function is variadic so if you want to send more information, you can just add more strings on and update the `argc` value. Also remember in the entry version of the function to *check the return value* and to free the entry arguments afterwards - this is only needed as the entry version is called when calling from the same executable and so the memory should be freed (this wouldn't happen when launching a separate executable and probably isn't needed in this case but it's good practice!)

The last thing of note is that we launch the library process with a call to [wimp_start_library_process(...)](https://docs.hdoc.io/billythesquid/WIMP/f84DC8AC67B51639C.html) which launched the function we specified as a new thread, separate from our current one. We supply the name of our child process (if the end process doesn't know it's name at compile time, this is a useful argument to add to the entry arguments), a casted function pointer to the entry point and a plibsys (one of the dependencies) enum signalling the thread priority - set this to normal for most cases. Add the entry variables at the end, and check for success, and you should get a message in the console letting you know the client has started. But our client now needs it's own server!

Luckily the process is essentially the same, except we need to check our command line arguments to get the port values. As these are just normal command line strings, you can access them however you want, however I'll use the disgusting C way of getting them (C++ users avert your eyes!):

    int client_main(int argc, char** argv)
    {
    	//Default values in case no arguments supplied
    	const char* process_domain = "127.0.0.1";
    	int32_t process_port = 8001;
    	const char* master_domain = "127.0.0.1";
    	int32_t master_port = 8000;
    
    	//Read the args in the C way
    	//We also check i + 1 < argc to ensure next argument was specified
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
    
    	return 0;
    }
This now gives us our port variables. Now, the next part involves both the client and the main processes so I'll show the new code for both at the same time:

**New main process code:**

    //Get the local server pointer
    WimpServer* server = wimp_get_local_server();
    
    //Start a reciever thread to recieve from our client
    RecieverArgs args = wimp_get_reciever_args("master", "127.0.0.1", process_port, &server->incomingmsg, &server->active);
    if (wimp_start_reciever_thread("client", "127.0.0.1", master_port, args) != WIMP_RECIEVER_SUCCESS)
    {
    	printf("Failed to start reciever thread!\n");
    	wimp_close_local_server();
    	wimp_shutdown();
    	return -4;
    }
    
    //Add the client process to the server process table
    wimp_process_table_add(&server->ptable, "client", "127.0.0.1", process_port, WIMP_Process_Child, NULL);
    
    //Accept the incoming connection from the client reciever
    //This is the reciever that writes to the client incoming queue
    if (wimp_server_process_accept(server, 1, "client") != WIMP_SERVER_SUCCESS)
    {
    	printf("Failed to accept connection from client reciever!\n");
    	wimp_close_local_server();
    	wimp_shutdown();
    	return -5;
    }
    
    printf("Successfully linked to client!\n");
    p_uthread_sleep(1000);
**New client process code:**

    //Same old initialization
    if (wimp_init() != WIMP_PROCESS_SUCCESS)
    {
    	return -1;
    }
    
    if (wimp_init_local_server("client", "127.0.0.1", process_port) != WIMP_SERVER_SUCCESS)
    {
    	printf("Failed to init the client local server!!!\n");
    	wimp_shutdown();
    	return -2;
    }
    printf("WIMP client succeeded on port: %d\n", process_port);
    
    //Get the client local server
    WimpServer* server = wimp_get_local_server();
    
    //Start a reciever thread to recieve from the master
    RecieverArgs args = wimp_get_reciever_args("client", "127.0.0.1", master_port, &server->incomingmsg, &server->active);
    if (wimp_start_reciever_thread("master", "127.0.0.1", process_port, args) != WIMP_RECIEVER_SUCCESS)
    {
    	printf("Failed to start client reciever thread!\n");
    	wimp_close_local_server();
    	wimp_shutdown();
    	return -4;
    }
    
    //Add the master process to the server process table
    wimp_process_table_add(&server->ptable, "master", "127.0.0.1", master_port, WIMP_Process_Parent, NULL);
    
    //Accept the incoming connection from the master reciever
    //This is the reciever that writes to the master incoming queue
    if (wimp_server_process_accept(server, 1, "master") != WIMP_SERVER_SUCCESS)
    {
    	printf("Failed to accept connection from master reciever!\n");
    	wimp_close_local_server();
    	wimp_shutdown();
    	return -5;
    }
    
    printf("Successfully linked to master!\n");
    p_uthread_sleep(1000);
    
    wimp_close_local_server();
    wimp_shutdown();
So we added a lot of new code. I'll break down the broad strokes of what we're doing before I explain the functions themselves.

Both the master and client want to start a reciever thread. This is essentially a thread that recieves instructions from a connected server, and adds them to the server's incoming queue so that it doesn't interrupt the potentially heavy task the server might be doing. This is how the instructions can be fire and forget. The master process starts a reciever that recieves from the client process, and vice versa. We then accept each process on each end which sends a handshake to each respective reciever and confirms the connection. We now have a relationship that looks like this:

![Untitled Diagram drawio](https://github.com/user-attachments/assets/98456c35-385a-494a-8a97-08a9ef352c10)
So our processes can send and recieve from each other now. But let's go into the actual code changes.

Both processes are mostly similar changes so I'll go over them together. For both processes we need to get the reciever arguments - as they are on a separate thread they also need some information to know where to look for their respective server connection. We call [wimp_get_reciever_args(...)](https://docs.hdoc.io/billythesquid/WIMP/f2BB36AD9FD0DED8F.html) and for each we specify the name of the process to write to (i.e. the process starting the thread), the domain to recieve from, the port to recieve from, the queue to write to and the active flag of the local server. To access those last two we get our local server pointer with [wimp_get_local_server()](https://docs.hdoc.io/billythesquid/WIMP/fDF892825DF51C8E8.html) and access the respective fields. We then start the reciever thread with [wimp_start_reciever_thread()](https://docs.hdoc.io/billythesquid/WIMP/fD3CE639BDC74A9D6.html) and specify the name of the process to recieve from, the domain the reciever writes to, the port the reciever writes to, and the reciever args previously made. We then use [wimp_process_table_add()](https://docs.hdoc.io/billythesquid/WIMP/f68A93A01FC416282.html) to add the connection to the respective other process to the server process table - a table that contains the information of the server connections - and specify the relation - `WIMP_Process_Child` for the master process's connection to the child and `WIMP_Process_Parent` for the child's connection to the master process. *Finally* we accept the connection on each end with [wimp_server_process_accept(...)](https://docs.hdoc.io/billythesquid/WIMP/f246FCFF51E90C1C5.html) - making sure to specify the number of connections to accept as multiple could be accepted with a variadic parameter - and the two processes are linked. The program should now print that both sides connected successfully!

### Simple connection loop

So we have the simplest possible connection, but how do we send instructions? We simply use [wimp_add_local_server(...)](https://docs.hdoc.io/billythesquid/WIMP/f4693CB6EB14AF573.html) to add an instruction to the server outgoing queue (directed at the target server) and [wimp_server_send_instructions(...)](https://docs.hdoc.io/billythesquid/WIMP/fB084685A0237FB1C.html) to send them - this means we can queue a lot of instructions up before sending. On the client side, let's send some arbitrary instructions to the master process:

    //Add instructions
    wimp_add_local_server("master", "blank_instr", NULL, 0);
    wimp_add_local_server("master", "say_hello", NULL, 0);
    
    const  char*  echo_string  =  "Echo!";
    wimp_add_local_server("master", "echo", echo_string, (strlen(echo_string) +  1) *  sizeof(char));
    wimp_log_success("Sent instructions!\n");
    wimp_add_local_server("master", WIMP_INSTRUCTION_EXIT, NULL, 0);
    wimp_server_send_instructions(server);
    p_uthread_sleep(1000);

Here we just send some arbitrary instructions - plus the special WIMP exit instruction at the end. In [wimp_add_local_server(...)](https://docs.hdoc.io/billythesquid/WIMP/f4693CB6EB14AF573.html) we simply specify the destination server, the string of the instruction, and a pointer to the arguments (if any exist) and their size. We then send with [wimp_server_send_instructions(server)](https://docs.hdoc.io/billythesquid/WIMP/fB084685A0237FB1C.html) using our local server pointer from earlier, and the instructions are sent to the master server! And if you were wondering, the special exit instruction tells the recieving server to close, and closes the recievers neatly in the process - any user defined exit command wouldn't shut the recievers down! We also use the **wimp_log** set of functions now - they act like printf except once a parent is connected they print the message to the parent process console and neatly tag them with the process name - *as long as _DEBUG is defined!* If it isn't, it won't print anything so feel free to use printf instead if you prefer.

Then, on the master process side, we need to recieve these instructions:

    //Simple loop
    bool disconnect = false;
    while (!disconnect)
    {
    	wimp_instr_queue_high_prio_lock(&server->incomingmsg);
    	WimpInstrNode currentnode = wimp_instr_queue_pop(&server->incomingmsg);
    	while (currentnode != NULL)
    	{
    		WimpInstrMeta meta = wimp_instr_get_from_node(currentnode);
    		if (wimp_instr_check(meta.instr, "blank_instr"))
    		{
    			wimp_log("\n");
    		}
    		else if (wimp_instr_check(meta.instr, "say_hello"))
    		{
    			wimp_log("HELLO!\n");
    		}
    		else if (wimp_instr_check(meta.instr, "echo"))
    		{
    			//Get the arguments
    			const char* echo_string = (const char*)meta.args;
    			wimp_log("%s\n", echo_string);
    		}
    		else if (wimp_instr_check(meta.instr, WIMP_INSTRUCTION_LOG))
    		{
    			wimp_log(meta.args);
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
    }

There are two loops in this situation: the outer program loop, and the inner instruction read loop. The program loop repeats until the exit signal is recieved. For each iteration of the program loop, the incoming instruction queue is locked (to prevent the reciever thread sneakily writing to it!), and the first instruction node is popped from the queue with [wimp_instr_queue_pop(&server->incomingmsg)](https://docs.hdoc.io/billythesquid/WIMP/fEC31211799FE536B.html). This instruction node contains the information of one instruction, and so if one wasn't sent, it will be NULL and the inner loop is skipped. The inner loop just keeps popping the next instruction until we have no more. Popping the node also implicitly passes us ownership so we need to free each one when we're done. Then we extract the instruction data from the node with [wimp_instr_get_from_node](https://docs.hdoc.io/billythesquid/WIMP/f5E47EA49A9889DF6.html) which lets us access the instruction data as if they were fields in a struct (actually they are laid out in a contiguous block of memory but as the field sizes aren't known at compile time accessing them any other way is a nightmare!). We can then check each instruction with [wimp_instr_check(...)](https://docs.hdoc.io/billythesquid/WIMP/f92171AE6E06C8E17.html) and perform the desired action. We add log in to the checking as when a child process uses [wimp_log(...)](https://docs.hdoc.io/billythesquid/WIMP/f6187A2C6E54E453F.html) it sends the log string to it's parent (and it's parents parent, and so on) to make sure the logs are all printed to the same console - so we need to check and print the message somewhere!

This should give a console output that looks something like this:

    WIMP Init
    Server created! master 127.0.0.1:39319
    WIMP succeeded on port: 39319
    Starting client!
    Starting Reciever for master recieving from client
    Server master waiting to accept 1 connections
    Server created! client 127.0.0.1:39479
    WIMP client succeeded on port: 39479
    Starting Reciever for client recieving from master
    master reciever failed to connect - trying again...
    Server client waiting to accept 1 connections
    client reciever connection at 127.0.0.1:39319
    Valid process found: client
    Adding to master process table: client
    Process added!
    master found every process!
    Successfully linked to client!
    master reciever failed to connect - trying again...
    master reciever connection at 127.0.0.1:39479
    Valid process found: master
    Adding to client process table: master
    Process added!
    Successfully linked to master!
    [client] client found every process!
    
    HELLO!
    Echo!
    [client] Sent instructions!
And that's it! A simple connection between two processes than can send instructions! The full code is located [here](https://github.com/BillyTheSquid21/wimp/blob/master/examples/hello-wimp.c) if you want to see it all together.

### Next Steps

With this set up, the next steps can be to make the client also operate as a loop so it can also recieve from the master. Currently this is the only tutorial, but the [tests folder](https://github.com/BillyTheSquid21/wimp/tree/master/tests) in the main repository contains some other usage examples for different features with some documentation.

