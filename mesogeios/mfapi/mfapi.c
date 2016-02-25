#include "mfapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

long fsize(const char *filename)
{
	struct stat st;
	if (stat(filename, &st) == -1)
		return -1;
	return st.st_size;
}

long ftob(const char *filename, char *buf, size_t n)
{
	FILE *f;
	long bread = 0;

	if (!(f = fopen(filename, "rb")))
		return -1;

	bread = fread(buf, n, 1, f);
	fclose(f);

	return bread;
}

long btof(const char *filename, char *buf, size_t n)
{
	FILE *f;
	long bwrite = 0;

	if (!(f = fopen(filename, "wb")))
		return -1;

	bwrite = fwrite(buf, n, 1, f);
	fclose(f);

	return bwrite;
}

void mmap_file(const char *filename, void **buf, size_t n, int *fd)
{
	if((*fd = open(filename, O_RDONLY)) < 0) {
		*fd = -1;
		*buf = NULL;
	} else {
		*buf = mmap(0, n, PROT_READ, MAP_PRIVATE, *fd, 0);
	}

}

void unmmap_file(void *buf, size_t n, int fd)
{
	munmap(buf, n);
	close(fd);
}


/* sample mains ------------------
	int main(int c, char *v[])
	{
		size_t size = fsize(v[1]);
		char *buf = (char *) malloc(size + 1);
		memset(buf, 0, size + 1);
		char *map; int fd;
		size_t i = 0;

		printf("%ld\n", size);
		printf("%ld\n", ftob(v[1], buf, size));
		printf("%s\n", buf);

		mmap_file(v[1], (void **)&map, size, &fd);
		for (; i < size; i++) {
			printf("%c", map[i]);
		}

		free(buf);
		return 0;
	}

	#include <string.h>

	int main(int c, char *v[]) {
		char *map;
		int fd;
		size_t size = fsize(v[1]);
		mmap_file(v[1], (void **)&map, size, &fd);
		printf("%ld\n", btof("test", map, size));
		unmmap_file(map, size, fd);
		return 0;
	}
 *------------------ sample mains */
