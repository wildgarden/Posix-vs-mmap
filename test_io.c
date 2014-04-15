#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <malloc.h>

int main(void)
{
	int fd;
	char *buf, *buf1;
	ssize_t ret;

	buf = malloc(4096 * 6);
	memset(buf, 'd', 4096 * 6);
	buf1 = malloc(4096 * 6);
	memset(buf1, 'a', 4096 * 6);

	fd = open("/mnt/ramdisk/test1", O_RDWR | O_CREAT, 0640);

	printf("Posix write to file: 0 - 5\n");
	ret = pwrite(fd, buf, 4096 * 6, 0);
	printf("Posix write to file: 0 - 5 ret: %d\n", ret);

	printf("Posix read from file: 2 - 6\n");
	ret = pread(fd, buf1, 4096 * 5, 4096 * 2);
	printf("Posix read from file: 2 - 6 ret: %d, buf: %c\n", ret, buf1[0]);

	memset(buf, 'f', 4096 * 6);
	printf("Posix write to file: 1 - 3\n");
	ret = pwrite(fd, buf, 4096 * 3, 4096);
	printf("Posix write to file: 1 - 3 ret: %d\n", ret);

	memset(buf, 'e', 4096 * 6);
	printf("Posix write to file: 4 - 8\n");
	ret = pwrite(fd, buf, 4096 * 5, 4096 * 4);
	printf("Posix write to file: 4 - 8 ret: %d\n", ret);

	printf("Posix read from file: 2 - 5\n");
	ret = pread(fd, buf1, 4096 * 4, 4096 * 2);
	printf("Posix read from file: 2 - 5 ret: %d, buf: %c %c %c %c\n",
		ret, buf1[0], buf1[4096], buf1[8192], buf1[4096 * 3]);

	close(fd);
	free(buf);
	free(buf1);

	return 0;
}