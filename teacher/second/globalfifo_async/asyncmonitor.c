/*======================================================================
    A test program to access /dev/second
    This example is to help understand async IO 
    
    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

  int fd = 0, oflags;
  unsigned char buf[0x10];
/*���յ��첽���źź�Ķ���*/
void input_handler(int signum)
{
	int count =0;
	int i =0;
#if 0
  char buff[100];
  fgets(buff, 40, stdin);
  printf("recevic buffer: %s\n", buff);
#endif
  printf("receive a signal from globalfifo,signalnum:%d\n",signum);
  count = read(fd, buf,  0x10 );
  for(i = 0; i <count; i++)
  printf("%c",buf[i] );
  printf("\n");
}

main()
{
  fd = open("/dev/globalfifo", O_RDWR );
  if (fd !=  - 1)
  {
    //�����ź���������
    signal(SIGIO, input_handler); //��input_handler()����SIGIO�ź�
#if 1
    fcntl(fd, F_SETOWN, getpid());
    oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | FASYNC);
#endif
#if 0
    fcntl(0, F_SETOWN, getpid());
    oflags = fcntl(0, F_GETFL);
    fcntl(0, F_SETFL, oflags | FASYNC);
#endif
    while(1)
    {
    	sleep(10);
		printf("wakeup \n");
    }
  }
  else
  {
    printf("device open failure\n");
  }
}
