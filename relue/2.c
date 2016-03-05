#include <stdio.h>
#include <stdbool.h>

static const long limit = 4000000; /* 4 million */

static bool
is_even(long n)
{
	return !(n % 2);
}

int
main(void)
{
	long summa = 2;
	for (long f_n2 = 1, f_n1 = 2, f_n = 3;
	     f_n < limit;
	     f_n2 = f_n1, f_n1 = f_n, f_n = f_n1 + f_n2)
		if (is_even(f_n))
			summa += f_n;
	printf("%ld\n", summa);
	return 0;
}

