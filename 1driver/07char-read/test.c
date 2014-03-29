#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char buf[50];
	int n;
	int fd = open("/dev/mydev",O_RDWR);

	if(fd<0)
	{
		perror("readopen");
		exit(-1);
	}

	printf("mydev open fd:%d\n",fd);
	memset(buf,0,sizeof(buf));
	printf("Read return: %d\n",n=read(fd,buf,sizeof(buf)-1));
	if(n==-1){
		perror("read-read");
	}
	buf[n] = '\0';
	printf("read buf:%s \n",buf);
	close(fd);
	printf("mydev closed!\n");

	return 0;
}
