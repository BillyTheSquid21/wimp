#ifndef WIMP_LOG_H
#define WIMP_LOG_H

#include <utility/thread_local.h>
#include <plibsys.h>

#define MAXIMUM_LOG_BYTES 4096 //Completely arbitrary atm to make it easy to push into buffer

/*
* Logs based on state of thread - ensures all logs should be on a master terminal
* Can also provide future functionality to log file instead of terminal
*/

/*
* Provides log functionality that can be switched
* 
* @param format The string (wimp_log format)
* @param ... The args
*/
void wimp_log(const char* format, ...);

#endif