#include <stdint.h>
#include <stdio.h>

#define CycleCmp(x, op, y)	((int16_t) ((x) - (y)) op 0)
//#define CycleCmp(x, op, y)	(((x) op (y)))

int
main(int c, char *v[])
{
    uint16_t i = UINT16_MAX, x = UINT16_MAX - (uint16_t)1;
    printf("%5u op %5u | >:%d <:%d =:%d \n",
           i, x,
           CycleCmp(i, >, x),
           CycleCmp(i, <, x),
           CycleCmp(i, ==, x));

    i = UINT16_MAX + (uint16_t)1; x = UINT16_MAX;
    printf("%5u op %5u | >:%d <:%d =:%d \n",
           i, x,
           CycleCmp(i, >, x),
           CycleCmp(i, <, x),
           CycleCmp(i, ==, x));

    i = UINT16_MAX + (uint16_t)2; x = UINT16_MAX;
    printf("%5u op %5u | >:%d <:%d =:%d \n",
           i, x,
           CycleCmp(i, >, x),
           CycleCmp(i, <, x),
           CycleCmp(i, ==, x));

    i = UINT16_MAX + (uint16_t)3; x = UINT16_MAX;
    printf("%5u op %5u | >:%d <:%d =:%d \n",
           i, x,
           CycleCmp(i, >, x),
           CycleCmp(i, <, x),
           CycleCmp(i, ==, x));

    i = INT16_MAX + (uint16_t)1; x = INT16_MAX;
    printf("%5u op %5u | >:%d <:%d =:%d \n",
           i, x,
           CycleCmp(i, >, x),
           CycleCmp(i, <, x),
           CycleCmp(i, ==, x));

    i = INT16_MAX + (uint16_t)2; x = INT16_MAX;
    printf("%5u op %5u | >:%d <:%d =:%d \n",
           i, x,
           CycleCmp(i, >, x),
           CycleCmp(i, <, x),
           CycleCmp(i, ==, x));

    i = INT16_MAX + (uint16_t)3; x = INT16_MAX;
    printf("%5u op %5u | >:%d <:%d =:%d \n",
           i, x,
           CycleCmp(i, >, x),
           CycleCmp(i, <, x),
           CycleCmp(i, ==, x));

    return 0;
}
