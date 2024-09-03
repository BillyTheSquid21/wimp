
# Overview

Will's Interesting Message Protocol (WIMP) is designed to make it easier to make modular programs with many decoupled processes. It relies on plibsys for platform independent sockets, threads and shared memory. It works by each process having at least one associated "server" which recieves instructions from other servers by way of a "reciever" process, one for each process that the server is directly connected to. Servers and recievers are simply TCP sockets with some additional features, such as a instruction queues. A simple system with a master process and two other processes might look like this:

![Simple drawio(5)](https://github.com/user-attachments/assets/223626e3-2ba0-49cf-a092-27eaea5e2c24)

(Reciever names are written as "source-destination-reciever")

This structure, allows the processes themselves to avoid blocking as much as possible, with the reciever threads waiting for instructions to be recieved from a server and then queuing them for its destination process to execute when it is available. This also allows the processes to act in a "fire and forget" fashion, and send instructions without waiting on results.

Features:
- Process creation and communication
  - Launching process as a thread (C and C++ only) with a specified priority
  - Launching process as a separate binary (can be any source that uses the shared library)
  - Processes can send arbitary string based instructions between each other (up to the user to sanitize)
- Parent/Child process relationships
  - When a process server is cleaned, it automatically instructs all of its children to exit as well
  - A process can opt to poll its parent for its status in case of a crash preventing the exit signals being sent
  - Instructions can be routed if a process sends it to a destination that it has no direct connection with (reduces the amount of reciever threads needed)
- Shared memory Space
  - Can opt to use a shared memory space, split into named slots
  - Memory slots can be reserved in any size and are accessed through an arena structure
  - Thread safe reading/writing, and generated arena pointers can be passed to any process if needed and will be valid
- Simplified logging
  - Logging will all show up on the same terminal window, with a tag identifying the message's process

Planned Features:
- Custon script writing/reading/execution system
- Redirecting log messages to a log file in release mode
- Privilege system for processes

# How to use

Full documentation is available [here](https://billythesquid21.github.io/wimp/).

For getting started, check out the guide [here](https://billythesquid21.github.io/wimp/docGetting-Started.html)

### Other notes

The tests folder of the repository contains examples of code, which can be built as their own executables. It is recommended at this stage to look at these, as they provide more depth than the above information. Most of the library functions and header files are also documented.

# Building

Should first clone the repository with ```git clone --recursive``` to ensure the dependencies are included.

Windows
---

Ninja (Preferred):

Prerequisite: Ensure you have gcc c compiler and ninja installed and set to path, cd to root directory of project

1. mkdir build && cd build
2. cmake .. -DCMAKE_CXX_COMPILER=gcc -G "Ninja"
3. cmake --build . -j10

MSVC (Visual Studio 16 2019 preferred, any version should work however):

Prerequisite: Ensure you have a visual studio sdk installed, cd to root directory of project

1. mkdir build && cd build
2. cmake .. -G "Visual Studio 16 2019"
3. Use the produced solution file to compile, setting the startup project to the desired project if building tests

Linux
---

Ninja (Preferred):

1. mkdir build && cd build
2. cmake .. -DCMAKE_CXX_COMPILER=gcc -G "Ninja"
3. cmake --build . -j10

Alternatively to step 3, there is a .vscode folder with launch options added for tests.

Additional Flags
---

To build the tests:
-DWIMP_BUILD_TESTS=1

