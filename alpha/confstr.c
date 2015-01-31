#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * Hopefully the number of options
 * available does not exceed the limit
 * below.
 */
#define MAX_OPTIONS 100

int main (int argc, char *argv[]) {
	int i;	
	long retval;
	char buf[BUFSIZ];

	if (argc < 2) {

		for(i = 0; i < MAX_OPTIONS ; i++) {
				retval = confstr(i, buf, BUFSIZ);

				if (retval == 0) {
					if (errno == 0)
						printf("%d invalid\n", i);
				} else {
					printf("%d %s\n", i, buf);
				}
		}

	} else {
		for (i = 1; i < argc; i++) {
				retval = confstr(atoi(argv[i]), buf, BUFSIZ);

				if (retval == 0) {
					if (errno == 0)
						printf("invalid\n");
					else
						printf("undefined\n");
				} else {
					printf("%s\n", buf);
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
