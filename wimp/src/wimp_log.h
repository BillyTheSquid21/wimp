///
/// @file Header that defines the WIMP log interface
///

#ifndef WIMP_LOG_H
#define WIMP_LOG_H

#include <utility/thread_local.h>
#include <plibsys.h>
#include <wimp_core.h>

#define MAXIMUM_LOG_BYTES 4096 //Completely arbitrary atm to make it easy to push into buffer
#define LOG_DIRECTION_BEHAVIOR TRUE //Turns on or off the directing logs (e.g. to master or a log file)

///
/// Logs based on state of thread - ensures all logs should be on a master terminal
/// Can also provide future functionality to log file instead of terminal
///

///
/// @brief Provides log functionality
///
/// @param format The string (wimp_log format)
/// @param ... The args
///
WIMP_API void wimp_log(const char* format, ...);

///
/// @brief Log functionality for failures
///
/// Colored red to make easy to spot in console
///
/// @param format The string (wimp_log format)
/// @param ... The args
///
WIMP_API void wimp_log_fail(const char* format, ...);

///
/// @brief Log functionality for successes
///
/// Colored green to make easy to spot in console
///
/// @param format The string (wimp_log format)
/// @param ... The args
///
WIMP_API void wimp_log_success(const char* format, ...);

///
/// @brief Log functionality for important lines
///
/// Colored blue to make easy to spot in console
///
/// @param format The string (wimp_log format)
/// @param ... The args
///
WIMP_API void wimp_log_important(const char* format, ...);

#endif