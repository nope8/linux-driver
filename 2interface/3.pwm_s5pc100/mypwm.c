/*
 * a sample for led 
 * author:lewis
 * test method: ./test and the light flash flow!
 * */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/errno.h> 		//for memset
#include <linux/ioctl.h>
#include <asm/system.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/wait.h> 		//for kfree kmalloc
#include <mach/regs-gpio.h>
#include <linux/ioport.h>
#include "ioctl.h"

#define GLOBALFIFO_SIZE 0x10 	//全局fifo最大4K字节
#define MEM_CLEAR 0x1  			//清零全局内存
#define GLOBALFIFO_MAJOE 250 	//预设globalfifo的主设备号

#define S5PC100_GPDCON 0xE0300080
#define S5PC100_TIMER_BASE 0xEA000000
#define S5PC100_TCFG0 0x0
#define S5PC100_TCFG1 0x4
#define S5PC100_TCON 0x8
#define S5PC100_TCNTB1 0x18
#define S5PC100_TCMTB1 0x1c

void __iomem *gpdcon;
void __iomem *timer_base;

MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
dev_t devno;

/*定义设备结构体*/
struct mypwm{
	struct cdev cdev;
	struct resource res;
}mypwm;

static void pwm_init(void)
{
	/* set pin to PWMTOUT1 mode*/
	writel((readl(gpdcon) & (~0xf<<4)) | (0x2<<4), gpdcon);
	/*设置预分频值*/
	writel(readl(timer_base+S5PC100_TCFG0) | 0xff, timer_base+S5PC100_TCFG0);	
	/*设置分频值*/
	writel((readl(timer_base+S5PC100_TCFG1) & ~(0xf<<4)) | 0x1<<4, timer_base+S5PC100_TCFG1);

	/*设置占空比*/
	writel(200,timer_base+S5PC100_TCNTB1);		//TCNTB：timer count buffer register   定时器计数寄存器
	writel(100,timer_base+S5PC100_TCMTB1);		//TCMPB: timer compare buffer register 定时器比较寄存器
	/*使能TOUT_1*/
	writel((readl(timer_base+S5PC100_TCON) & ~(0xf<<8)) | (0x2<<8), timer_base+S5PC100_TCON); //TCON: time control register 定时器控制寄存器
}

static void pwm_on(void)
{
	writel((readl(timer_base+S5PC100_TCON) & ~(0xf<<8))|0x9<<8, timer_base+S5PC100_TCON);
}

static void pwm_off(void)
{
	writel((readl(timer_base+S5PC100_TCON) & ~(0xf<<8)), timer_base+S5PC100_TCON);
}

static void pwm_cnt(unsigned long arg)
{
	writel((readl(timer_base+S5PC100_TCON) & ~(0xf<<8)), timer_base+S5PC100_TCON);
	writel(arg, timer_base+S5PC100_TCNTB1);
	writel(arg>>1, timer_base+S5PC100_TCMTB1);
	writel((readl(timer_base+S5PC100_TCON) & ~(0xf<<8))|0x9<<8, timer_base+S5PC100_TCON);
}

static void pwm_pre(unsigned long arg)
{
	writel((readl(timer_base+S5PC100_TCON) & ~(0xf<<8)), timer_base+S5PC100_TCON);
	writel(readl(timer_base+S5PC100_TCFG0) | arg, timer_base+S5PC100_TCFG0);
	writel((readl(timer_base+S5PC100_TCON) & ~(0xf<<8))|0x9<<8, timer_base+S5PC100_TCON);
}

int pwm_open(struct inode *inode, struct file *filp)
{
	pwm_init();
	printk(KERN_INFO "test if it arrive here! 86\n");
	printk(KERN_INFO "pwm opened\n");
	return 0;
}

int pwm_release(struct inode *inode, struct file *filp)
{
	writel((readl(timer_base+S5PC100_TCON) & ~(0xf<<8)), timer_base+S5PC100_TCON);
	printk(KERN_INFO "pwm closed\n");
	return 0;
}

static int pwm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
	case PWM_ON:
		pwm_on();
		break;
	case PWM_OFF:
		pwm_off();
		break;
	case PWM_PRE:
		pwm_pre(arg);
		break;
	case PWM_CNT:
		pwm_cnt(arg);
		break;
	default:
		printk(KERN_WARNING "wrong input\n");
		break;
	}
	return 0;
}

/*文件操作结构体*/
struct file_operations myfops = {
	.owner = THIS_MODULE,
	.open = pwm_open,
	.release = pwm_release,
	.ioctl = pwm_ioctl
};

void cdev_setup_cdev(void)
{
	int error;

//	devno = MKDEV(major,minor);
	cdev_init(&mypwm.cdev,&myfops);
	error = cdev_add(&mypwm.cdev,devno,1);
	if(error < 0)
		printk(KERN_WARNING "Error: %d adding pwm device\n",error);
	printk(KERN_INFO "mypwm registered\n");
}

static int __init s5pc100_pwm_init(void)
{
	devno = MKDEV(major,minor);
	register_chrdev_region(devno,count,"pwm");
	cdev_setup_cdev();
	
	if(request_mem_region(S5PC100_GPDCON,0x4,"gpdcon") == NULL)
	{
		printk(KERN_INFO "request failed\n");
		return -1;
	}
	/* I/O内存资源申请,并检查申请的资源是否可用 */
	if(request_mem_region(S5PC100_TIMER_BASE,0x30,"timer_base") == NULL)
	{
		printk(KERN_INFO "request failed\n");
		return -1;
	}

	gpdcon = ioremap(S5PC100_GPDCON,0x4);
	timer_base = ioremap(S5PC100_TIMER_BASE,0x30);
	return 0;
}

static void __exit s5pc100_pwm_cleanup(void)
{
	iounmap(gpdcon);
	iounmap(timer_base);

	/* 释放申请I/O内存资源 */
	release_mem_region(S5PC100_GPDCON,0x4);
	release_mem_region(S5PC100_TIMER_BASE,0x30);

	cdev_del(&mypwm.cdev);
	unregister_chrdev_region(devno,count);
	printk(KERN_INFO "mypwm unregistered!\n");
}

module_init (s5pc100_pwm_init);
module_exit (s5pc100_pwm_cleanup);
