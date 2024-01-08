#ifndef WIMP_TEST_H
#define WIMP_TEST_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/timeb.h>

//Timer struct
typedef struct _TTimer
{
	struct timeb s;
	struct timeb e;
} TTimer;

//Test passing matrix
typedef struct _PASSMAT
{
	const char* step;
	bool status;
	TTimer timer;
} PASSMAT;

/*
* Init the timer
*/
TTimer timer_init();

/*
* Starts the timer
*/
void timer_start(TTimer* t);

/*
* Ends the timer
*/
void timer_end(TTimer* t);

/*
* Gets the time elapsed
*/
float get_time_elapsed(const TTimer t);

/*
* Validates test results
*/
bool wimp_test_validate_passmat(PASSMAT* matrix, size_t entries);

#endif