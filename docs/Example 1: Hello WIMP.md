
### Hello WIMP

Creating a WIMP system is pretty simple, but first you'll want to ensure everything is setup correctly. This tutorial is for using WIMP in C/C++ and so will use CMake - this is currently the only supported build automation tool, although I'm sure an enterprising individual can use the project with alternative systems such as MAKE.

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
(insert on github)


