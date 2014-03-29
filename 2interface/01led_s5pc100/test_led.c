#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/*ioctl define*/
#define LED_FLAG 'k'
#define LED_ON _IO(LED_FLAG,0)
#define LED_OFF _IO(LED_FLAG,1)

int main(int argc, const char *argv[])
{
	int fd;
	unsigned long i = 0x1;

	fd = open("/dev/myled",O_RDWR);

	if(fd < 0)
	{
		perror("open");
		printf("i can't open /dev/myled!\n");
		exit(-1);
	}

	while(1)
	{
		ioctl(fd,LED_ON,i);
		usleep(100000);
		ioctl(fd,LED_OFF,i);
		usleep(100000);
		i = i << 1;
		if(i == 16)
			i = 1;
	}

	return 0;
}

