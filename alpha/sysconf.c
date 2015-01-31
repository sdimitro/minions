#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * Hopefully the number of options
 * available does not exceed the limit
 * below.
 */
#define MAX_OPTIONS 500

int main (int argc, char *argv[]) {
	int i;	
	long retval;

	if (argc < 2) {

		for(i = 0; i < MAX_OPTIONS ; i++) {
			retval = sysconf(i);

			if (retval == -1) {
				if (errno == 0)
					printf("%d unlimited\n", i);
			} else {
				printf("%d %ld\n", i, retval);
			}
		}
	} else {
		for (i = 1; i < argc; i++) {
			retval = sysconf(atoi(argv[i]));

			if (retval == -1) {
				if (errno == 0)
					printf("unlimited\n");
				else
					printf("undefined\n");
			} else {
				printf("%ld\n", retval);
			}
		}
	}

	return 0;
}

/*
 * TODO:
 * Maybe one day I can add do the tedious
 * job of adding the option of printing out
 * the name of the limit name or the limit code
 * next to each limit. Or maybe have the name
 * of the limit as the argument. Sounds very 
 * tedious though.
 */
