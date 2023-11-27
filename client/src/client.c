#include <wimp_reciever.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_MESSAGE_LENGTH 1024
#define SERVER_PORT 5432

int main(int argc, char** argv)
{
	p_libsys_init();
	wimp_reciever_recieve("127.0.0.1", SERVER_PORT);
	p_libsys_shutdown();
	return 0;
}