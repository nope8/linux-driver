#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <asm-generic/ioctl.h>
#include "ioctl.h"
#include <string.h>
#include <linux/types.h>
#include <sys/select.h>

int main(void)
{
	fd_set rfds,wfds;
	int count;

	char buf[64] = "abcde123456789ghijklmnopksdfasdfasdfg";
	char cmd[64];
	int fd = open("/dev/mydev",O_RDWR);
	if(fd < 0)
	{
		perror("readopen");
		exit(-1);
	}
	printf("mydev open fd:%d\n",fd);

//	ioctl(fd,DEV_ONE);
//	ioctl(fd,DEV_TWO);

	while(1)
	{
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_SET(fd,&rfds);
		FD_SET(fd,&wfds);

		select(fd+1,&rfds,&wfds,NULL,NULL);
		/*数据可获得*/
		if(FD_ISSET(fd,&rfds))
		{
			printf("Poll monitor: can be read!!\n");
			memset(cmd,0,sizeof(cmd));
			if((count=read(fd,cmd,12)) < 0 )
			{
				perror("read");
				exit(-1);
			}
			printf("read from kernel %d byte \n\n\n",count);
		}
		/*数据可写入*/
		if(FD_ISSET(fd,&wfds))
		{
			printf("Poll monitor: can be write!!\n");
			if((count = write(fd,buf,strlen(buf))) == -1)
			{
				perror("write");
				exit(-1);
			}
			printf("write to kernel %d byte \n\n\n",count);
		}
		sleep(1);
	}

#if 0
	if(write(fd,buf,sizeof(buf))< 0)
	{
		perror("write");
	}
	
//	memset(buf,0,sizeof(buf));
//	read(fd,buf,6);
#endif
	close(fd);
	printf("mydev closed!\n");

	return 0;
}
