#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <asm-generic/ioctl.h>
#include <string.h>

int main(void)
{
	char buf[64] = "hello joy!!!!";

	int fd = open("/dev/mydev",O_RDWR);

	if(fd<0)
	{
		perror("readopen");
		exit(-1);
	}

	printf("mydev open fd:%d\n",fd);
	
	if(write(fd,buf,sizeof(buf)-1)< 0)
	{
		perror("write");
		exit(EXIT_FAILURE);
	}
	int fd1 = open("/dev/mydev",O_RDWR);
	memset(buf,0,sizeof(buf));
	if(read(fd,buf,5) < 0)
	{
		perror("read");
		exit(EXIT_FAILURE);
	}
	printf("returned buf:%s\n",buf);

	close(fd);
	printf("mydev closed!\n");

	return 0;
}
