#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char buf[] = "write:happy everyday write to kernel!!";
	int n;
	int fd = open("/dev/mydev",O_RDWR);
	
	if(fd < 0){
		perror("open");
		exit(-1);
	}
	printf("mydev opened %d\n",fd);
//	memset(buf,0,sizeof(buf));
	printf("write returns %d \n",n=write(fd,buf,sizeof(buf)-1));
	close(fd);
//	printf("mydev closed!\n");

	return 0;
}
