#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

int major=250,minor=0;
int count = 1,result;

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

	printk(KERN_INFO "mydev register success!\n");
	return 0;
}



static void __exit hello_cleanup(void)
{
	dev_t udev;

	udev = MKDEV(major,minor);
	unregister_chrdev_region(udev,count);
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);

