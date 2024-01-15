#include <wimp_test.h>

TTimer timer_init()
{
	TTimer t;
	t.s.time = 0;
	t.s.millitm = 0;
	t.e.time = 0;
	t.e.millitm = 0;
	return t;
}

void timer_start(TTimer* t)
{
	ftime(&t->s);
}

void timer_end(TTimer* t)
{
	ftime(&t->e);
}

float get_time_elapsed(const TTimer t)
{
	long sec_diff = t.e.time - t.s.time;
	short mill_diff = t.e.millitm - t.s.millitm;

	float secs = (float)sec_diff;
	float mills = ((float)mill_diff) / 1000.0f;
	return secs + mills;
}

bool wimp_test_validate_passmat(PASSMAT* matrix, size_t entries)
{
	bool haspassed = true;

	wimp_log("\n\n\nTest Results:\n");

	for (size_t i = 0; i < entries; ++i)
	{
		PASSMAT m = matrix[i];
		haspassed &= m.status;
		printf("%s: %s", m.step, m.status ? "PASSED" : "FAILED");
		if (m.timer.s.time != 0 && m.timer.e.time != 0)
		{
			printf(" %f\s", get_time_elapsed(m.timer));
		}
		printf("\n");
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