#ifndef WIMP_DEBUG_H
#define WIMP_DEBUG_H

//Make the debug macro work regardless of env
#if defined _DEBUG || defined DEBUG

#define WIMP_DEBUG

//Can add debug utility functions here - might make a debug print

#else

#endif
#endif