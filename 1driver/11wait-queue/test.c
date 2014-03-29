#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <asm-generic/ioctl.h>
#include "ioctl.h"
#include <string.h>
int main(void)
{
	char buf[64] = "I am a great Chinese Linux driver Engineer";
	int fd = open("/dev/mydev",O_RDWR);

	if(fd<0)
	{
		perror("readopen");
		exit(-1);
	}

	printf("mydev open fd:%d\n",fd);

//	ioctl(fd,DEV_ONE);
//	ioctl(fd,DEV_TWO);

	if(write(fd,buf,sizeof(buf))< 0)
	{
		perror("write");
	}
	
//	memset(buf,0,sizeof(buf));
//	read(fd,buf,6);

	close(fd);
	printf("mydev closed!\n");

	return 0;
}
