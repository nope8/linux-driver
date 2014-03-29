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

#define GLOBALFIFO_SIZE 0x10 	//全局fifo最大4K字节
#define MEM_CLEAR 0x1  			//清零全局内存
#define GLOBALFIFO_MAJOE 250 	//预设globalfifo的主设备号
/*ioctl define*/
#define BEEP_FLAG 'k'
#define BEEP_ON _IO(BEEP_FLAG,0)
#define BEEP_OFF _IO(BEEP_FLAG,1)

MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
dev_t devno;

/*定义设备机构体*/
struct mybeep{
	struct cdev cdev;	
}mybeep;

static void beep_init(void)
{
	printk(KERN_INFO "test if it arrive here!\n");
	writel((readl(S5PC100_GPD_BASE) & (~0xf<<4)) | (0x1<<4), S5PC100_GPD_BASE);

}

static void beep_on(unsigned long arg)
{
	writel((readl(S5PC100_GPG3_BASE+4) | 0x1<<1), S5PC100_GPD_BASE+4);
}

static void beep_off(unsigned long arg)
{
	writel((readl(S5PC100_GPG3_BASE+4) & (~0x1<<1)), S5PC100_GPD_BASE+4);
}

int beep_open(struct inode *inode, struct file *filp)
{
	beep_init();
	printk(KERN_INFO "test if it arrive here! 62\n");
	printk(KERN_INFO "beep opened\n");
	return 0;
}

int beep_release(struct inode *inode, struct file *filp)
{
	writel((readl(S5PC100_GPG3_BASE+4) & (~0xf)), S5PC100_GPG3_BASE);
	printk(KERN_INFO "beep closed\n");
	return 0;
}

static int beep_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
	case BEEP_ON:
		beep_on(arg);
		break;
	case BEEP_OFF:
		beep_off(arg);
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
	.open = beep_open,
	.release = beep_release,
	.ioctl = beep_ioctl
};

void cdev_setup_cdev(void)
{
	int error;

	devno = MKDEV(major,minor);
	cdev_init(&mybeep.cdev,&myfops);
	error = cdev_add(&mybeep.cdev,devno,1);
	if(error < 0)
		printk(KERN_WARNING "Error: %d adding beep device\n",error);
	printk(KERN_INFO "mybeep registered\n");
}

static int __init s5pc100_beep_init(void)
{
	devno = MKDEV(major,minor);
	register_chrdev_region(devno,count,"myled");
	cdev_setup_cdev();
	
	return 0;
}

static void __exit s5pc100_beep_cleanup(void)
{
	cdev_del(&mybeep.cdev);
	unregister_chrdev_region(devno,count);
	printk(KERN_INFO "mybeep unregistered!\n");
}

module_init (s5pc100_beep_init);
module_exit (s5pc100_beep_cleanup);
