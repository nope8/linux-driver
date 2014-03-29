#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <asm-generic/ioctl.h>
#include "ioctl.h"
#include <string.h>
#include <linux/types.h>
#include <sys/select.h>
#include <signal.h>
#define MAX_LEN 64
int fd = 0;
#if 0
void intput_handler(int signum)
{
	char buf[MAX_LEN];
	int len,count,i;

	fd = open("/dev/mydev",O_RDWR);
	printf("receive a signal from mydev,signum %d\n",signum);
	count = read(fd,buf,MAX_LEN);
	for(i=0; i<count; i++)
	{
		printf("%c",buf[i]);
	}
}
#endif
int main(void)
{
	int oflags,fd;
	int count = 0, old_count = 0;

	fd = open("/dev/mydev",O_RDONLY);
	if(fd < 0)
	{
		perror("open");
		exit(-1);
	}
#if 0
	signal(SIGIO,intput_handler);
	fcntl(0,F_SETOWN,getpid());
	oflags = fcntl(0,F_GETFL);
	fcntl(0,F_SETFL,oflags|FASYNC);
#endif
	while(1)
	{
		read(fd,&count,sizeof(unsigned int));
		if(count != old_count)
		{
			printf("seconds after open /dev/mydev:%d\n",count);
			old_count = count;
		}
	}

	close(fd);
	return 0;
}
