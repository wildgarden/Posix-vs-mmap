#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<sys/mman.h>
#include<string.h>
#include<pthread.h>

#define END_SIZE	(64UL * 1024 * 1024) 

const int start_size = 512;

char* memcpy1(char *to, char *from, size_t n)
{
	long esi, edi;
	int ecx;
	esi = (long)from;
	edi = (long)to;
	asm volatile("rep ; movsl"
		: "=&c" (ecx), "=&D" (edi), "=&S" (esi)
		: "0" (n / 4), "1" (edi), "2" (esi)
		: "memory"
		);
	return to;
}

#define MMX2_MEMCPY_MIN_LEN 0x40
#define MMX_MMREG_SIZE 8

// ftp://ftp.work.acer-euro.com/gpl/AS1800/xine-lib/src/xine-utils/memcpy.c
static void * mmx2_memcpy(void * __restrict__ to, const void * __restrict__ from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  /* PREFETCH has effect even for MOVSB instruction ;) */
  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
    "   prefetchnta 288(%0)\n"
    : : "r" (from) );

  if(len >= MMX2_MEMCPY_MIN_LEN)
  {
    register unsigned long int delta;
    /* Align destinition to MMREG_SIZE -boundary */
    delta = ((unsigned long int)to)&(MMX_MMREG_SIZE-1);
    if(delta)
    {
      delta=MMX_MMREG_SIZE-delta;
      len -= delta;
      memcpy(to, from, delta);
    }
    i = len >> 6; /* len/64 */
    len&=63;
    for(; i>0; i--)
    {
      __asm__ __volatile__ (
      "prefetchnta 320(%0)\n"
      "prefetchnta 352(%0)\n"
      "movq (%0), %%mm0\n"
      "movq 8(%0), %%mm1\n"
      "movq 16(%0), %%mm2\n"
      "movq 24(%0), %%mm3\n"
      "movq 32(%0), %%mm4\n"
      "movq 40(%0), %%mm5\n"
      "movq 48(%0), %%mm6\n"
      "movq 56(%0), %%mm7\n"
      "movntq %%mm0, (%1)\n"
      "movntq %%mm1, 8(%1)\n"
      "movntq %%mm2, 16(%1)\n"
      "movntq %%mm3, 24(%1)\n"
      "movntq %%mm4, 32(%1)\n"
      "movntq %%mm5, 40(%1)\n"
      "movntq %%mm6, 48(%1)\n"
      "movntq %%mm7, 56(%1)\n"
      :: "r" (from), "r" (to) : "memory");
      //((const unsigned char *)from)+=64;
      //((unsigned char *)to)+=64;
      from = (void*)(((const unsigned char *)from) + 64);
      to = (void*)(((unsigned char *)to) + 64);
    }
     /* since movntq is weakly-ordered, a "sfence"
     * is needed to become ordered again. */
    __asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("emms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */
  if(len) memcpy(to, from, len);
  return retval;
}

int main(int argc, char ** argv)
{
	int fd, i;
	long long time;
	unsigned long long FILE_SIZE;
	size_t len;
	char c = 'A';
	char unit;
	char *origin_data;
	char *data;
	struct timespec start, end;
	void *buf1 = NULL;
	char *buf;
	int size, count;
	FILE *output;
	char fs_type[20];
	char file_size_num[20];
	char xip_enabled[20];
	char use_nvp[20];
	char filename[60];

	if (argc < 6) {
		printf("Usage: ./mmap_to_ram $FS $XIP $Quill $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);

	if (!strcmp(argv[2], "0"))
		strcpy(xip_enabled, "No_XIP");
	else
		strcpy(xip_enabled, "XIP");

	if (!strcmp(argv[3], "0"))
		strcpy(use_nvp, "Posix-mmap");
	else
		strcpy(use_nvp, "Quill-mmap");

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

	if (FILE_SIZE < END_SIZE)
		FILE_SIZE = END_SIZE;
	if (FILE_SIZE > 2147483648) // RAM disk size
		FILE_SIZE = 2147483648;

	strcpy(filename, argv[5]);
	output = fopen(filename, "a");

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) {
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR | O_DIRECT, 0640); 
	data = (char *)mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	origin_data = data;

	for (size = start_size; size <= END_SIZE; size <<= 1) {
		memset(buf, c, size);
		c++;
		data = origin_data;
		count = FILE_SIZE / size;

		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++) {
			mmx2_memcpy(data, buf, size);
			data += size;
		}

		msync(origin_data, FILE_SIZE, MS_ASYNC);
		clock_gettime(CLOCK_MONOTONIC, &end);

		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("Mmap: size %d bytes, %d times,\t %lld nanoseconds,\t Bandwidth %f MB/s.\n", size, count, time, FILE_SIZE * 1024.0 / time);
		fprintf(output, "%s,%s,%s,%d,%lld,%d,%lld,%f\n", fs_type, use_nvp, xip_enabled, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time);
	}

	fclose(output);
	munmap(origin_data, FILE_SIZE);
	close(fd);
	free(buf1);
	return 0;
}
