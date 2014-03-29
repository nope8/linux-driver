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
#include <linux/errno.h>
//for memset
#include <linux/ioctl.h>
#include <asm/system.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include "ioctl.h"
#include <linux/slab.h>
//for kfree kmalloc
MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
//char data[128] = "char_read: the message for reading!!!";
dev_t devno;

//spinlock_t *lock;
//struct cdev cdev;

struct mydevice{
	char data[128];
	struct cdev cdev;	
	struct semaphore sem;
};

struct mydevice *mydevice;

static int dev_open(struct inode *inode, struct file *filp){
	filp->private_data = mydevice;
	printk(KERN_INFO "mydev opened!\n");
	return 0;
}

static int dev_release(struct inode *inode, struct file *filp){
	printk(KERN_INFO "mydev closed!\n");
	return 0;
}

static int dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	size_t ret;
	struct mydevice *dev = filp->private_data;

	if(count>127) 	count=127;
	if(count<0) 	return -EINVAL;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if(copy_to_user(buf,dev->data,count)){
		ret = -EFAULT;
	}
	else{
		printk(KERN_INFO "read from kernel %d Bytes\n",count);
		ret = count;
	}
	printk(KERN_INFO "ret = %d\n",ret);
	up(&dev->sem);
	return ret;
}

static int dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	int ret;
	struct mydevice *dev = filp->private_data;
	printk(KERN_INFO "write %d Bytes",count);

	if(count>127) 	return -ENOMEM;
	if(count<0) 	return -EINVAL;
	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	msleep(10000);
	if(copy_from_user(dev->data,buf,count))
	{
		ret = -EFAULT;
	}
	else
	{
		dev->data[count] = '\0';
		printk(KERN_INFO "receiving from user %s \n",dev->data);
		ret = count;
	}
	up(&mydevice->sem);
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

static void char_setup_cdev(struct mydevice *dev)
{
	int error;
	devno = MKDEV(major,minor);
//	dev->cdev = cdev_alloc();
	cdev_init(&dev->cdev,&myfops);
	error = cdev_add(&dev->cdev,devno,1);
	if(error < 0)
	{
		printk(KERN_WARNING "Error:%d adding char dev!\n",error);
	}
	printk(KERN_INFO "dynamic cdev registed\n");
}

static int  __init hello_init(void)
{
	int result;
	devno = MKDEV(major,minor);
	result = register_chrdev_region(devno,count,"mydev");
	if(result != 0)
	{
		printk(KERN_WARNING "mydev:can't get major num %d!\n",major);
		return result;
	}

	mydevice = (struct mydevice *)kmalloc(sizeof(struct mydevice),GFP_KERNEL);
	if(!mydevice){
		result = -ENOMEM;
		printk(KERN_WARNING "fail malloc!\n");
		goto fail_malloc;
	}

	memset(mydevice,0,sizeof(struct mydevice));
	char_setup_cdev(mydevice);
	sema_init(&mydevice->sem,1);
	printk(KERN_INFO "mydev register success!\n");
	
	return 0;

fail_malloc: unregister_chrdev_region(devno,count);
	return result;
}

static void __exit hello_cleanup(void)
{
	devno = MKDEV(major,minor);
	cdev_del(&mydevice->cdev);
	kfree(mydevice);
	unregister_chrdev_region(devno,count);
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);

