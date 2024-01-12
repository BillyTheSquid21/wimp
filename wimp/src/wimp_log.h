#ifndef WIMP_LOG_H
#define WIMP_LOG_H

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