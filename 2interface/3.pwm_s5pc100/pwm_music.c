/*
 * main.c : test demo driver
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "pwm_music.h"
#include "ioctl.h"
//#include "s5pc100_pwm.h"

int main()
{
	int i = 0;
	int n = 2;
	int dev_fd;
	int div;
	dev_fd = open("/dev/pwm",O_RDWR | O_NONBLOCK);
	if (dev_fd == -1)
	{
		perror("open");
		exit(-1);
	}

	ioctl(dev_fd,PWM_PRE,255);

	//ioctl(dev_fd,PWM_CNT,1);

	for(i = 0;i<sizeof(FishBoat)/sizeof(Note);i++ )
	{
		div = (PCLK/256/4)/(FishBoat[i].pitch);
		ioctl(dev_fd, PWM_CNT, div);
		usleep(FishBoat[i].dimation * 100); 
	}

	for(i = 0;i<sizeof(MumIsTheBestInTheWorld)/sizeof(Note);i++ )
	{
		div = (PCLK/256/4)/(MumIsTheBestInTheWorld[i].pitch);
		ioctl(dev_fd, PWM_CNT, div);
		usleep(MumIsTheBestInTheWorld[i].dimation * 100); 
	}

	for(i = 0;i<sizeof(GreatlyLongNow)/sizeof(Note);i++ )
	{
		div = (PCLK/256/4)/(GreatlyLongNow[i].pitch);
		ioctl(dev_fd, PWM_CNT, div);
		usleep(GreatlyLongNow[i].dimation * 100); 
	}
	return 0;
}
