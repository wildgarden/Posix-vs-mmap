#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>

#define SIZE		(4 * 1024 * 1024) 
#define FILE_SIZE	(4 * 1024 * 1024) 

const int start_size = 2097152;

int main(void)
{
	int fd, i;
	unsigned long time;
	char c = 'a';
	struct timespec start, end;
	int size, count;
	void *buf1 = NULL;
	char *buf;

	posix_memalign(&buf1, FILE_SIZE, FILE_SIZE); // up to 1MB
	if (!buf1) {
		printf("ERROR!\n");
		return 0;
	}

	buf = (char *)buf1;

	for (size = start_size; size <= FILE_SIZE; size <<= 1) {
		memset(buf, c, size);
		c++;

		fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR | O_DIRECT); 

		count = FILE_SIZE / size;
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++)
			write(fd, buf, size);

		clock_gettime(CLOCK_MONOTONIC, &end);
		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("Size %d bytes,\t %ld nanoseconds,\t Bandwidth %f MB/s.\n", size, time, 4.0 * 1e9 / time);
		close(fd);
	}

	free(buf1);
	return 0;
}
