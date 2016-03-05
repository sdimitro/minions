#include <stdio.h>

long long target = 1000000000;

long long
sdb(int n)
{
	long long p = target / n;
	return n * (p * (p + 1)) / 2;
}

int
main(void)
{
	long long result = sdb(3) + sdb(5) - sdb(15);
	printf("%lld\n", result);
	return 0;
}

