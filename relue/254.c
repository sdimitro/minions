#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static const int M = 150;
static const int N = 63;
static const int fact9 = 362880;


/* Reads as "is x greater than y?" */
inline bool
gt(int x[], int y[])
{
	for (int i = 1; i < 9; i++)
		if (x[i] != y[i])
			return x[i] < y[i];
	return false;
}

int
main(void)
{
	// TODO: maybe hardcode them?
	// precompute n! and 10^n.
	long long p10[19];
	int fac[9];
	fac[0] = p10[0] = 1;
	for (int i = 1; i < 9 ; i++) fac[i] = i * fac[i-1];
	for (int i = 1; i < 19; i++) p10[i] = 10 * p10[i-1];
	
	// TODO: maybe do one malloc for all?
	/* precompute h(x) = min f^(-1)(x), for x <- 0 to 9! */
	int (* dh)[9] = malloc(sizeof(int) * fact9 * 9); /* # of each digit */
	int (* sh) = malloc(sizeof(int) * fact9);        /* sum of digits */
	int (* lh) = malloc(sizeof(int) * fact9);        /* length in digits */

	for (int x = 0; x < fact9; x++) {
		sh[x] = lh[x] = 0; int t = x;
		for (int i = 8; i > 0; i--) {
			dh[x][i] = t / fac[i];
			sh[x] += i * dh[x][i];
			lh[x] += dh[x][i];
			t %= fac[i];
		}
	}

	/* brute force g(i) for all i < 63 with all x < 10^7 */
	int g[N][9]; /* # each digits */
	int lg[N];   /* length in digits */
	int d[8] = {0, 0, 0, 0, 0, 0, 0, 0}; /* actual digits */

	for (int x = 0; x < N; x++) lg[x] = 0;

	/* x = a * 9! + b, s = s(x) */
	for (int x = 0, a = 0, b = 0, s = 0;
	     x < p10[7];
	     x++, b++, s++, d[0]++) {

		/* if b got to 9! just reset it
		 * and increment a */
		if (b == fact9) {
			b = 0;
			a++;
		}

		for (int i = 0; i < 7 && 9 < d[i]; i++) {
			d[i] = 0;
			d[i+1]++;
			s -= 9;   /* carry up */
		}

		if (s >= N)
			continue;

		if (lg[s] == 0 ||
		    lg[s] > a + lh[b] ||
		    (lg[s] == a + lh[b] && gt(g[s], dh[b]))) {
			lg[s] = a + lh[b];
			for (int i = 1; i < 9; i++) g[s][i] = dh[b][i];
		}
	}
	
	/* sum precomputed sg(i)'s */
	long long s = 0;
	for (int x = 1; x < N; x++) {
		s += 9 * lg[x];
		for (int i = 8; i > 0; i--) s -= i * g[x][9-i];
	}

	/* compute the rest */
	for(int x = N; x <= M; x++) {
		long long t = (x % 9 + 1) * p10[x / 9] - 1;
		s += (t / fact9) * 9 + sh[t % fact9];
	}

	printf("%lld\n", s);

	/* free allocated data */
	free(sh);
	free(dh);
	free(lh);

	return 0;
}
