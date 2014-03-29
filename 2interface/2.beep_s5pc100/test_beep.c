#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/*ioctl define*/
#define BEEP_FLAG 'k'
#define BEEP_ON _IO(BEEP_FLAG,0)
#define BEEP_OFF _IO(BEEP_FLAG,1)

int main(int argc, const char *argv[])
{
	int fd;
	unsigned long i = 0x1;

	fd = open("/dev/mybeep",O_RDWR);

	if(fd < 0)
	{
		perror("open");
		printf("i can't open /dev/mybeep!\n");
		exit(-1);
	}

	while(1)
	{
		ioctl(fd,BEEP_ON,i);
		usleep(100000);
		ioctl(fd,BEEP_OFF,i);
		usleep(100000);
		i = i << 1;
		if(i == 16)
			i = 1;
	}

	return 0;
}

