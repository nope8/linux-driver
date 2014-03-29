#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/sem.h>

int fd;
void signal_handler(int signo)
{
	int key;

	read(fd,&key,sizeof(key));
	printf("press key %d \n",key);

}

int main(int argc, const char *argv[])
{
	
	int oflags;

	if((fd = open("/dev/key",O_RDONLY)) < 0)
	{
		perror("open");
		exit(-1);
	}

	fcntl(fd,F_SETOWN,getpid());
	oflags = fcntl(fd,F_GETFL);
	fcntl(fd,F_SETFL,oflags | FASYNC);
	
	signal(SIGIO,signal_handler);
		
	while(1)
	{
		sleep(1);
	}

	return 0;
}
