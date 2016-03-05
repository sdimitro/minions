#include <stdio.h>

static const long limit = 4000000; /* 4 million */

int
main(void)
{
	long summa = 0;
	long f_n2 = 1, f_n1 = 1, f_n = 2;
	while(f_n < limit) {
		summa += f_n;
		f_n2 = f_n1 + f_n;
		f_n1 = f_n2 + f_n;
		f_n  = f_n1 + f_n2;
	}
	printf("%ld\n", summa);
	return 0;
}

