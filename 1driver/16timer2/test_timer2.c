#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, const char *argv[])
{
	unsigned long counter = 0, old_counter = 0;
	int fd = open("/dev/mydev",O_RDONLY);
	if(fd < 0)
	{
		perror("open");
		exit(-1);
	}

	while(1)
	{
		if(read(fd,&counter,sizeof(unsigned int)) < 0)
		{
			perror("read");
			exit(-1);
		}
		if(counter != old_counter)
		{
			printf("seconds after open /dev/mydev:%ld\n",counter);
			old_counter = counter;
		}
	}
	return 0;
}
