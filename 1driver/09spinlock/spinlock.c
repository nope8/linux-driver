#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include "spinlock.h"

MODULE_LICENSE("GPL");
	

int major=250,minor=0;
int count = 1;
dev_t dev;
struct lewis_device{
	int flag;
	char data[128];
	spinlock_t lock;
	struct cdev cdev;
}mydev;
spinlock_t t_lock;

static int dev_open(struct inode *inode, struct file *file){
	spin_lock(&mydev.lock);
	if(!mydev.flag)
	{
		printk(KERN_WARNING "spinlock opened\n");
		return -EBUSY;
	}
	mydev.flag --;
	spin_unlock(&mydev.lock);
	printk(KERN_INFO "mydev opened!\n");
	return 0;
}

static int dev_release(struct inode *inode, struct file *file){
	spin_lock(&mydev.lock);
	mydev.flag ++;
	spin_unlock(&mydev.lock);
	printk(KERN_INFO "mydev closed!\n");
	return 0;
}

static int dev_read(struct file *fp, char __user *buf, size_t count, loff_t *offset)
{
	size_t ret;

	if(count>127) 	count=127;
	if(count<0) 	return -EINVAL;

	if(copy_to_user(buf,mydev.data,count)){
		ret = -EFAULT;
	}
	else{
		printk(KERN_INFO "read from kernel %d Bytes\n",count);
		ret = count;
	}
	return ret;
}

static int dev_write(struct file *fp, const char __user *buf, size_t count, loff_t *offset){
	int ret;
	
	spin_lock(&t_lock);
	printk("write lock\n");
	msleep(20000);
	printk(KERN_INFO "write %d Bytes",count);
	if(count>127) 	return -ENOMEM;
	if(count<0) 	return -EINVAL;
	
	if(copy_from_user(mydev.data,buf,count)){
		ret = -EFAULT;
	}
	else{
		mydev.data[count] = '\0';
		printk(KERN_INFO "receiving from user %s\n",mydev.data);
		ret = count;
	}

	spin_unlock(&t_lock);
	printk("write unlock");
	return ret;
} 
long dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg){
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

static void char_reg_cdev(void){
	int error;
	
	dev = MKDEV(major,minor);
	cdev_init(&mydev.cdev,&myfops);
	error = cdev_add(&mydev.cdev,dev,1);
	if(error)
		printk(KERN_WARNING "Error:%d adding char dev!\n",error);
}

static int  __init hello_init(void){
	int result;

	dev = MKDEV(major,minor);
	result = register_chrdev_region(dev,count,"mydev");
	if(result < 0)
	{
		printk(KERN_WARNING "mydev:can't get major num %d!\n",major);
		return result;
	}
	char_reg_cdev();
	mydev.flag=1;
	spin_lock_init(&mydev.lock);
	spin_lock_init(&t_lock);

	printk(KERN_INFO "12345678\n");
	printk(KERN_INFO "char device registed\n");
	return 0;
}

static void __exit hello_cleanup(void)
{
	dev = MKDEV(major,minor);
	cdev_del(&mydev.cdev);
	unregister_chrdev_region(dev,count);
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);
