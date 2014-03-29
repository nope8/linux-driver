#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");

int major=250,minor=0;
int count = 1,result;
int error;

struct cdev cdev;
struct file_operations myfops = {
	.owner = THIS_MODULE
};

static int  __init hello_init(void)
{
	dev_t dev;
	dev = MKDEV(major,minor);
	result = register_chrdev_region(dev,count,"mydev");
	if(result != 0)
	{
		printk(KERN_WARNING "mydev:can't get major!\n");
		return result;
	}

	cdev_init(&cdev,&myfops);
	error = cdev_add(&cdev,dev,1);
	if(error < 0)
	{
		printk(KERN_WARNING "mydev:can't register cdev!\n");
		return error;
	}

	printk(KERN_INFO "cdev registed\n");
	printk(KERN_INFO "mydev register success!\n");
	return 0;
}

static void __exit hello_cleanup(void)
{
	dev_t udev;

	udev = MKDEV(major,minor);
	cdev_del(&cdev);
	unregister_chrdev_region(udev,count);
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);

EXPORT_SYMBOL(hello_init);
EXPORT_SYMBOL_GPL(hello_cleanup);
