#include <wimp_test.h>

bool wimp_test_validate_passmat(PASSMAT* matrix, size_t entries)
{
	bool haspassed = true;

	printf("\n\n\nTest Results:\n");

	for (size_t i = 0; i < entries; ++i)
	{
		PASSMAT m = matrix[i];
		haspassed &= m.status;
		printf("%s: %d\n", m.step, m.status);
	}

	if (haspassed)
	{
		printf("Test case has passed!\n");
	}
	else
	{
		printf("Test case has failed!\n");
	}

	return haspassed;
}