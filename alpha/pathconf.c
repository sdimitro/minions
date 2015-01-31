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
	int i, j;	
	long retval;
	char *path;

	if (argc < 2) {
		printf("Usage: pathconf filename [... filenames]\n");
		return -1;
	}
	
	for (i = 1; i < argc; i++) {
		path = argv[i];

		if ((retval = access(path, F_OK)) == -1) {
			printf("error %d: cannot open file %s\n", errno, path);
			continue;
		}
		
		for (j = 0; j < MAX_OPTIONS; j++) {
			retval = pathconf(path, j);

			if (retval == -1) {
				if (errno == 0)
					printf("%s %d unlimited\n", path, j);
			} else {
				printf("%s %d %ld\n", path, j, retval);
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
