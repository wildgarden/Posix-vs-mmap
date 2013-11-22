#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/time.h>

#define END_SIZE	(64UL * 1024 * 1024) 

const int start_size = 512;

int main(int argc, char **argv)
{
	int fd, i;
	long long time, time1;
	unsigned long long FILE_SIZE;
	size_t len;
	off_t offset;
	char c = 'a';
	char unit;
	struct timespec start, end;
	struct timeval begin, finish;
	struct timezone tz;
	int size;
	unsigned long long count;
	void *buf1 = NULL;
	char *buf;
	FILE *output;
	char fs_type[20];
	char quill_enabled[20];
	char file_size_num[20];
	char filename[60];
	int enable_ftrace;

#if 0
	if (argc < 6) {
		printf("Usage: ./write_to_ram $FS $QUILL $ENABLE_FTRACE $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);

	if (!strcmp(argv[2], "0"))
		strcpy(quill_enabled, "Posix");
	else
		strcpy(quill_enabled, "Quill");

	strcpy(file_size_num, argv[4]);
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

	strcpy(filename, argv[5]);
	output = fopen(filename, "a");
#endif

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	fd = open("/mnt/ramdisk/test2", O_CREAT | O_RDWR | O_DIRECT, 0640); 
//	fd = open("/dev/null", O_WRONLY, 0640); 
//	fd = open("/dev/zero", O_RDONLY, 0640); 
	printf("fd: %d\n", fd);
	sleep(10);
//	start_size = atoi(argv[2]);
//	enable_ftrace = atoi(argv[3]);
//	for (size = start_size; size <= END_SIZE; size <<= 1) {
//		size = 8192;

#if 0
		size = atoi(argv[2]);
		memset(buf, c, size);
		c++;
		lseek(fd, 0, SEEK_SET);
		offset = 0;
//#if 0
		count = 1073741824 / size;
		// Warm the cache with 1GB write
		gettimeofday(&begin, &tz);
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		for (i = 0; i < count; i++) {
			if (pwrite(fd, buf, size, offset) != size)
				printf("ERROR!\n");
			offset += size;
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);
		gettimeofday(&finish, &tz);
		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		time1 = (finish.tv_sec - begin.tv_sec) * 1e6 + (finish.tv_usec - begin.tv_usec);
		printf("Write warm: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t latency %lld nanoseconds, \t Bandwidth %f MB/s.\n", size, count, time, time / count, FILE_SIZE * 1024.0 / time);
		printf("Warm cache process %lld microseconds\n", time1);

//#endif
		lseek(fd, 0, SEEK_SET);
		count = FILE_SIZE / size;
		offset = 0;
		if (enable_ftrace)
			system("echo 1 > /sys/kernel/debug/tracing/tracing_on");
	
		gettimeofday(&begin, &tz);
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		for (i = 0; i < count; i++) {
			pwrite(fd, buf, size, offset);
//			if (pwrite(fd, buf, size, offset) != size)
//				printf("ERROR!\n");
			offset += size;
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);
		gettimeofday(&finish, &tz);

		if (enable_ftrace)
			system("echo 0 > /sys/kernel/debug/tracing/tracing_on");
		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		time1 = (finish.tv_sec - begin.tv_sec) * 1e6 + (finish.tv_usec - begin.tv_usec);
		printf("Write: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t latency %lld nanoseconds, \t Bandwidth %f MB/s.\n", size, count, time, time / count, FILE_SIZE * 1024.0 / time);
		printf("Write process %lld microseconds\n", time1);
		fprintf(output, "%s,%s,%d,%lld,%lld,%lld,%f,%lld\n", fs_type, quill_enabled, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time, time / count);
//	}
#endif

//	fclose(output);
	close(fd);
	free(buf1);
	return 0;
}