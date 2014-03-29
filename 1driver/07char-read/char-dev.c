#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
char data[128] = "char_read: the message for reading!!!";

struct cdev * cdev;

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
	return ret;

}

static int dev_write(struct file *fp, const char __user *buf, size_t count, loff_t *offset)
{
	int ret;

	printk(KERN_INFO "write %d Bytes",count);

	if(count>127) 	return -ENOMEM;
	if(count<0) 	return -EINVAL;

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
	return ret;
} 

struct file_operations myfops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,
	.read = dev_read,
	.write = dev_write
};

static void char_reg_cdev(void)
{
	int error;
	dev_t devno = MKDEV(major,minor);
	cdev = cdev_alloc();
	cdev_init(cdev,&myfops);
	error = cdev_add(cdev,devno,1);
	if(error < 0)
	{
		printk(KERN_WARNING "Error:%d adding char dev!\n",error);
	}
	printk(KERN_INFO "dynamic cdev registed\n");
}

static int  __init hello_init(void)
{
	int result;
	dev_t dev;
	dev = MKDEV(major,minor);
	result = register_chrdev_region(dev,count,"mydev");
	if(result != 0)
	{
		printk(KERN_WARNING "mydev:can't get major num %d!\n",major);
		return result;
	}
	char_reg_cdev();

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

