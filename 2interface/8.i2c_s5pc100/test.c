#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ioctl.h"

int main(int argc, const char *argv[])
{
	int fd;
	int num = 0,count=0;
	int high,low;

	fd = open("/dev/lm75",O_RDONLY);
	
	while(1)
	{
		if((count=read(fd,&num,4)) == -1)
		{
			perror("read");
			exit(-1);
		}
		low = num & 0xff;
		high = num>>8;
		printf("temp = %d.%d C\n",high,low);
		printf("%d\n",count);
		sleep(1);
	}

	close(fd);
	
	return 0;
}
