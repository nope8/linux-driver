#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;

struct cdev * cdev;

static int dev_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "mydev opened!\n");
	return 0;
}

static int dev_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "mydev closed!\n");
	return 0;
}

struct file_operations myfops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release
};

static int  __init hello_init(void)
{
	int result,error;
	dev_t dev;
	dev = MKDEV(major,minor);
	result = register_chrdev_region(dev,count,"mydev");
	if(result != 0)
	{
		printk(KERN_WARNING "mydev:can't get major!\n");
		return result;
	}
	cdev = cdev_alloc();
	cdev_init(cdev,&myfops);
	error = cdev_add(cdev,dev,1);
	if(error < 0)
	{
		printk(KERN_WARNING "Error:%d adding char dev!\n",error);
		return error;
	}
	printk(KERN_INFO "dynamic cdev registed\n");
	printk(KERN_INFO "mydev register success!\n");
	return 0;
}

static void __exit hello_cleanup(void)
{
	dev_t udev;
	
	udev = MKDEV(major,minor);
	if(udev)
		cdev_del(cdev);
	unregister_chrdev_region(udev,count);
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);

