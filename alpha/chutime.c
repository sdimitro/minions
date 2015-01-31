#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[]) {
	int ret_val = 0;

	/* variables/pointers to previous data */
	struct stat cstat;
	struct tm *ctmp;
	time_t *ctimep;

	/* new data variables */
	struct tm *ntmp;
	time_t ntime;
	struct utimbuf ninfo;

	if (argc < 5) {
		printf("Usage: chutime <filaname> <day> <month> <year>");
		return -1;
	}

	if ((ret_val = access(argv[1], F_OK)) == -1) {
		printf("error %d: cannot open file %s\n", errno, argv[1]);
		return ret_val;
	}


	if ((ret_val = stat(argv[1], &cstat)) == -1) {
		printf("error %d: failed on stat\n", errno);
		return ret_val;
	}

	ctimep = &(cstat.st_mtime);
	ctmp = localtime(ctimep);
	ntime = mktime(ctmp);
	ntmp = localtime(&ntime);

	ntmp->tm_mday = atoi(argv[2]);
	ntmp->tm_mon = atoi(argv[3]) - 1;
	ntmp->tm_year = atoi(argv[4]) - 1900;

	ntime = mktime(ntmp);
	ninfo.actime = ntime;
	ninfo.modtime = ntime;


	if ((ret_val = utime(argv[1], &ninfo)) == -1) {
		printf("error %d: failed on utime", errno);
		return ret_val;
	}

	return ret_val;
}
