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
void intput_handler(int signum)
{
	char buf[MAX_LEN];
	int len,count,i;

	printf("receive a signal from mydev,signum %d\n",signum);
	count = read(fd,buf,MAX_LEN);
	for(i=0; i<count; i++)
	{
		printf("%c",buf[i]);
	}
}

int main(void)
{
	int oflags,fd;

	fd = open("/dev/mydev",O_RDWR);

	signal(SIGIO,intput_handler);
	fcntl(fd,F_SETOWN,getpid());
	oflags = fcntl(fd,F_GETFL);
	fcntl(fd,F_SETFL,oflags|FASYNC);
	
	while(1)
	{
		sleep(10);
		printf("wakeup \n");
	}
	return 0;
}
