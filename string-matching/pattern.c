#include <stdio.h>
#include <string.h>

void print_prefix_result(char *s, int a[], int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("%2c ", s[i]);
	printf("\n");

	for (i = 0; i < len; i++)
		printf("%2d ", a[i]);
	printf("\n");
}

void clrs_compute_prefix(char *p)
{
	int m = strlen(p);
	int pi[m];
	pi[0] = -1;

	int q, k = 0;
	for (q = 1; q < m; q++) {
		pi[q] = k; /* line is originally at the end of the loop */
		while (k > 0 && p[k] != p[q])
			k = pi[k];
		if (p[k] == p[q])
			k++;
	}

	print_prefix_result(p, pi, m);
}

void clrs_calculate_comparisons(char *p)
{
	int comparisons = 0;

	int m = strlen(p);
	int pi[m];
	pi[0] = -1;
	int q, k = 0;

	comparisons++;
	for (q = 1; q < m; q++) {

		pi[q] = k;

		comparisons += (k > 0) ? 2 : 1;
		while (k > 0 && p[k] != p[q])
			k = pi[k];

		comparisons++;
		if (p[k] == p[q]) {
			k++;
		}

		comparisons++;
	}

	printf("total comparisons: %d\n", comparisons);
}

void s_compute_prefix(char *s)
{
	int len = strlen(s);
	int prefix[len];

	prefix[0] = -1;

	int i, j = 0;
	for (i = 1; i < len; i++) {
		prefix[i] = j;
		if (s[i] == s[j])
			j++;
		else
			j = 0;
	}

	print_prefix_result(s, prefix, len);
}

void s_calculate_comparisons(char *s)
{
	int comparisons = 0;
	int len = strlen(s);

	#define _unused(x) ((void)x)
	int prefix[len];

	_unused(prefix); /* makes compiler warning go away */
	prefix[0] = -1;

	int i, j = 0;

	comparisons++;
	for (i = 1; i < len; i++) {
		prefix[i] = j;

		comparisons++;
		if (s[i] == s[j])
			j++;
		else
			j = 0;

		comparisons++;
	}

	printf("total comparisons: %d\n", comparisons);
}

int main(int c, char *v[])
{
	char *pattern = "abcabcacab";

	if (c > 1)
		pattern = v[1];

	printf("Pattern: %s\n\n", pattern);

	clrs_compute_prefix(pattern);
	clrs_calculate_comparisons(pattern);

	printf("\n");

	s_compute_prefix(pattern);
	s_calculate_comparisons(pattern);

	return 0;
}
