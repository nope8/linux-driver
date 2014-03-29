#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <asm-generic/ioctl.h>
int main(void)
{
	char buf[] = "hello joy!!!!";

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
	
	if(read(fd,buf,8) < 0)
	{
		perror("read");
		exit(EXIT_FAILURE);
	}

	close(fd);
	printf("mydev closed!\n");

	return 0;
}
