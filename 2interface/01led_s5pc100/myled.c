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
#include <linux/slab.h> 		//for kfree kmalloc
#include <linux/wait.h> 		

#include <mach/regs-gpio.h>

#define GLOBALFIFO_SIZE 0x10 	//全局fifo最大4K字节
#define MEM_CLEAR 0x1  			//清零全局内存
#define GLOBALFIFO_MAJOE 250 	//预设globalfifo的主设备号
/*ioctl define*/
#define LED_FLAG 'k'
#define LED_ON _IO(LED_FLAG,0)
#define LED_OFF _IO(LED_FLAG,1)

MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
dev_t devno;

/*定义设备机构体*/
struct myled{
	struct cdev cdev;	
}myled;

static void led_init(void)
{
	printk(KERN_INFO "test if it arrive here!\n");
	writel((readl(S5PC100_GPG3_BASE) & (~0xffff)) | 0x1111, S5PC100_GPG3_BASE);

}

static void led_on(unsigned long arg)
{
	writel(readl(S5PC100_GPG3_BASE+4) | arg, S5PC100_GPG3_BASE+4);
}

static void led_off(unsigned long arg)
{
	writel(readl(S5PC100_GPG3_BASE+4) & (~arg), S5PC100_GPG3_BASE+4);
}

int led_open(struct inode *inode, struct file *filp)
{
	led_init();
	printk(KERN_INFO "test if it arrive here! 62\n");
	printk(KERN_INFO "led opened\n");
	return 0;
}

int led_release(struct inode *inode, struct file *filp)
{
	writel((readl(S5PC100_GPG3_BASE+4) & (~0xf)), S5PC100_GPG3_BASE);
	printk(KERN_INFO "led closed\n");
	return 0;
}

static int led_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
	case LED_ON:
		led_on(arg);
		break;
	case LED_OFF:
		led_off(arg);
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
	.open = led_open,
	.release = led_release,
	.ioctl = led_ioctl
};

void cdev_setup_cdev(void)
{
	int error;

	devno = MKDEV(major,minor);
	cdev_init(&myled.cdev,&myfops);
	error = cdev_add(&myled.cdev,devno,1);
	if(error < 0)
		printk(KERN_WARNING "Error: %d adding led device\n",error);
	printk(KERN_INFO "led registered\n");
}

static int __init s5pc100_led_init(void)
{
	devno = MKDEV(major,minor);
	register_chrdev_region(devno,count,"myled");
	cdev_setup_cdev();
	
	return 0;
}

static void __exit s5pc100_led_cleanup(void)
{
	cdev_del(&myled.cdev);
	unregister_chrdev_region(devno,count);
	printk(KERN_INFO "myled unregistered!\n");
}

module_init (s5pc100_led_init);
module_exit (s5pc100_led_cleanup);
