#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <plibsys.h>
#include <pmacros.h>
#include <ptypes.h>
#include <perrortypes.h>

#define MAX_PORT_CHARACTER_LENGTH 6
#define UNI_SOCKET_DEFAULT_SERVER_BUFLEN 512

//Define the core commands that interact with an exchange loop
//Very simple now, and won't be how I continue to do it
//Deliminate with \n for one word and " " for arguments
#define UNI_SOCKET_SHUTDOWN_COMMAND "shutdown\n"      //Disconnect from sending socket and shut down recieving socket (breaks loop)
#define UNI_SOCKET_DISCONNECT_COMMAND "disconnect\n"  //Disconnect from sending socket only
#define UNI_SOCKET_ECHO_COMMAND "echo "			      //Echos the text back to the user
#define UNI_SOCKET_UNKNOWN_REQUEST "unknown request!\n"

//Check if a command matches the recieved buffer
bool check_buffer_command(const char* buffer, const char* command, size_t buffer_len, size_t command_len);