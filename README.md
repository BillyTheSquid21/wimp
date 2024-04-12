# Overview
---

Will's Interesting Message Protocol (WIMP) is designed to make it easier to make modular programs with many decoupled processes. It relies on plibsys for platform independent sockets, threads and shared memory. It works by each process having at least one associated "server" which recieves instructions from other servers by way of a "reciever" process, one for each process that the server is directly connected to. A simple system with a master process and two other processes might look like this:

![Simple drawio(4)](https://github.com/BillyTheSquid21/Wills-Interesting-Message-Protocol/assets/97798337/fa7d79f9-8d31-4416-a8a0-3352fd36ff20)

(Reciever names are written as "source-destination-reciever")

This structure, allows the processes themselves to avoid blocking as much as possible, with the reciever threads waiting for instructions to be recieved from a server and then queuing them for its destination process to execute when it is available. This also allows the processes to act in a "fire and forget" fashion, and send instructions without waiting on results.

# Building

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
