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
	int fd;

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
	printf("fd: %d\n", fd);

	fsync(fd);
	close(fd);

	return 0;
}
