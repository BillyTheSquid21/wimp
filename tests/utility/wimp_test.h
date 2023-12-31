#ifndef WIMP_TEST_H
#define WIMP_TEST_H

#include <stdbool.h>
#include <stdio.h>

//Test passing matrix
typedef struct _PASSMAT
{
	const char* step;
	bool status;
} PASSMAT;

/*
* Validates test results
*/
bool wimp_test_validate_passmat(PASSMAT* matrix, size_t entries);

#endif