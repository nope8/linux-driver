#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <asm-generic/ioctl.h>
#include "ioctl.h"
int main(void)
{
	char buf[50];
	int fd = open("/dev/mydev",O_RDWR);

	if(fd<0)
	{
		perror("readopen");
		exit(-1);
	}

	printf("mydev open fd:%d\n",fd);

	ioctl(fd,DEV_ONE);
	ioctl(fd,DEV_TWO);

	close(fd);
	printf("mydev closed!\n");

	return 0;
}
