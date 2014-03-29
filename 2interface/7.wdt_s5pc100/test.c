#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ioctl.h"

int main(int argc, const char *argv[])
{
	int fd;
	int n = 10;

	fd = open("/dev/dog",O_RDWR);
	
	while(n--)
	{
		ioctl(fd,FEED_DOG);
		sleep(1);
		printf("%d\n",n);
	}

	sleep(30);

	close(fd);
	
	return 0;
}
