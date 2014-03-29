#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include "ioctl.h"

MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
char data[128] = "char_read: the message for reading!!!";
dev_t dev;
spinlock_t *lock;
struct cdev * cdev;
struct semaphore sem;



static int dev_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "mydev opened!\n");
	return 0;
}

static int dev_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "mydev closed!\n");
	return 0;
}

static int dev_read(struct file *fp, char __user *buf, size_t count, loff_t *offset)
{
	size_t ret;

	if(count>127) 	count=127;
	if(count<0) 	return -EINVAL;

	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	if(copy_to_user(buf,data,count))
	{
		ret = -EFAULT;
	}
	else
	{
		printk(KERN_INFO "read from kernel %d Bytes\n",count);
		ret = count;
	}
	printk(KERN_INFO "ret = %d\n",ret);
	up(&sem);
	return ret;

}

static int dev_write(struct file *fp, const char __user *buf, size_t count, loff_t *offset)
{
	int ret;

	printk(KERN_INFO "write %d Bytes",count);

	if(count>127) 	return -ENOMEM;
	if(count<0) 	return -EINVAL;
	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	msleep(10000);
	if(copy_from_user(data,buf,count))
	{
		ret = -EFAULT;
	}
	else
	{
		data[count] = '\0';
		printk(KERN_INFO "receiving from user %s \n",data);
		ret = count;
	}
	up(&sem);
	return ret;
} 

long dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	long ret=0;

	switch(cmd){
	case DEV_ONE:
		printk(KERN_INFO "one to go");
		break;
	case DEV_TWO:
		printk(KERN_INFO "two to go");
		break;
	default:
		break;
	}
	return ret;
}

struct file_operations myfops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,
	.read = dev_read,
	.write = dev_write,
	.unlocked_ioctl = dev_ioctl,
};

static void char_reg_cdev(void)
{
	int error;
	dev = MKDEV(major,minor);
//	cdev = cdev_alloc();
	cdev_init(cdev,&myfops);
	error = cdev_add(cdev,dev,1);
	if(error < 0)
	{
		printk(KERN_WARNING "Error:%d adding char dev!\n",error);
	}
	printk(KERN_INFO "dynamic cdev registed\n");

	
}

static int  __init hello_init(void)
{
	int result;
	dev = MKDEV(major,minor);
	result = register_chrdev_region(dev,count,"mydev");
	if(result != 0)
	{
		printk(KERN_WARNING "mydev:can't get major num %d!\n",major);
		return result;
	}
	char_reg_cdev();
	sema_init(&sem,1);
	printk(KERN_INFO "mydev register success!\n");
	

	return 0;
}

static void __exit hello_cleanup(void)
{
	dev = MKDEV(major,minor);
	cdev_del(cdev);
	unregister_chrdev_region(dev,count);
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);

