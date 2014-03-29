#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, const char *argv[])
{
	int data;
	int fd = open("/dev/adc",O_RDONLY);
	if(fd < 0)
	{
		perror("open");
		exit(-1);
	}

	while(1)
	{
		read(fd,(char *)&data,4);
		printf("data:%d\n",data);
		printf("voltage:%lf\n\n",3.3/4096*data);

		sleep(1);
	}

	close(fd);
	printf("/dev/adc/ closed!\n");
	return 0;
}
