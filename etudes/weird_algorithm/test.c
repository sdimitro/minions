#include <stdio.h>
#include <stdlib.h>

int
main(int c, char *v[])
{
    int val = atoi("3");

    while (val != 1) {
        printf("%d -> ", val);
        if ((val % 2) == 0) {
            val /= 2;
        } else {
            val = (val * 3) + 1;
        }
    }
    printf("%d\n", val);
}