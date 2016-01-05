#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/time.h>

#define END_SIZE	(4UL * 1024 * 1024) 

const int start_size = 512;

int main(int argc, char **argv)
{
	int fd, i, j;
	unsigned long long FILE_SIZE;
	size_t len;
	char c;
	char unit;
	int size;
	unsigned long long count;
	void *buf1 = NULL;
	char *buf;
	char file_size_num[20];
	int req_size;
	struct timespec begin, finish;
	int retry;
	long long time1;

	if (argc < 4) {
		printf("Usage: ./rewrite $REQ_SIZE $FILE_SIZE $REWRITE_TIME\n");
		return 0;
	}

	strcpy(file_size_num, argv[2]);
	len = strlen(file_size_num);
	unit = file_size_num[len - 1];
	file_size_num[len - 1] = '\0';
	FILE_SIZE = atoll(file_size_num);
	switch (unit) {
	case 'K':
	case 'k':
		FILE_SIZE *= 1024;
		break;
	case 'M':
	case 'm':
		FILE_SIZE *= 1048576;
		break;
	case 'G':
	case 'g':
		FILE_SIZE *= 1073741824;
		break;
	default:
		printf("ERROR: FILE_SIZE should be #K/M/G format.\n");
		return 0;
		break;
	}

	if (FILE_SIZE < 4096)
		FILE_SIZE = 4096;
	if (FILE_SIZE > 2147483648) // RAM disk size
		FILE_SIZE = 2147483648;

	retry = atoi(argv[3]);
	c = 'a';

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
	printf("fd: %d\n", fd);
	req_size = atoi(argv[1]);
	if (req_size > END_SIZE)
		req_size = END_SIZE;

	size = req_size;
	memset(buf, c, size);
	count = FILE_SIZE / size;

	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (i = 0; i < retry; i++) {
		lseek(fd, 0, SEEK_SET);

		for (j = 0; j < count; j++) {
			if (write(fd, buf, size) != size)
				printf("ERROR\n");
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);

	time1 = (finish.tv_sec * 1e9 + finish.tv_nsec) - (begin.tv_sec * 1e9 + begin.tv_nsec);
	printf("write %lld ns, average %lld ns\n", time1, time1 / (count * retry));

//	fsync(fd);
	close(fd);

	free(buf1);
	return 0;
}