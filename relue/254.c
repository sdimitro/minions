#include <stdio.h>
#include <stdbool.h>

// const int M = 150;
// const int N = 63;
// const int fac9 = 362880;
#define FACT9 362880


/* Reads as "is x greater than y?" */
//inline bool
//gt(int x[], int y[])
//{
//	for (int i = 1; i < 9; i++)
//		if (x[i] != y[i])
//			return x[i] < y[i];
//	return false;
//}

int
main(void)
{
	// precalculate n! and 10^n.
	//long long p10[19];
	int fac[9];
	fac[0] = 1;// p10[0] = 1;
	
	for (int i = 1; i < 9 ; i++) fac[i] = i * fac[i-1];
	//for (int i = 1; i < 19; i++) p10[i] = 10 * p10[i-1];

	
	// precalculate h(x) = min f^(-1)(x), 0 to 9!
	// dh := # each digits, sh := sum of digits, lh := length
	int sdh[FACT9][9];
	int ssh[FACT9];
	int slh[FACT9];

	
	//int (* dh)[9] = sdh;
	//int (* sh) = ssh;
	//int (* lh) = slh;

	for (int x = 0; x < FACT9; x++) {
		ssh[x] = slh[x] = 0; int t = x;
		for (int i = 8; i > 0; i--) {
			sdh[x][i] = t / fac[i];
			ssh[x] += i * sdh[x][i];
			slh[x] += sdh[x][i];
			t %= fac[i];
		}
	}
	printf("%d\n", sdh[0][0]);
	/*

	// brute force for g(i), x < 10^7, i < 63
	int g[N][9];
	int lg[N];
	int d[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // # each digits, length, digits of x
	for (int x = 0; x < N; x++) lg[x] = 0;

	for (int x = 0, a = 0, b = 0, s = 0;
	     x < p10[7];
	     x++, b++, s++, d[0]++) {// x = a * 9! + b, s = s(x)
		if (b == fac9) {
			b = 0;
			a++; // # 9!
		}

		for (int i = 0; i < 7 && 9 < d[i]; i++) {
			d[i] = 0;
			d[i+1]++;
			s -= 9; // carry up
		}

		if (s >= N)
			continue;
		if (lg[s] == 0 ||
		    lg[s] > a + slh[b] ||
		    (lg[s] == a + slh[b] && gt(g[s], sdh[b]))) {
			lg[s] = a + slh[b];
			for (int i = 1; i < 9; i++) g[s][i] = sdh[b][i];
		}
	}
	
	// sum up sg(i)
	long long s = 0;
	for (int x = 1; x < N; x++) {
		s += 9 * lg[x];
		for (int i = 8; i > 0; i--) s -= i * g[x][9-i];
	}

	for(int x = N; x <= M; x++) {
		long long t = (x % 9 + 1) * p10[x / 9] - 1;
		s += (t / fac9) * 9 + ssh[t % fac9];
	}
	printf("%lld\n", s);
	*/
}
