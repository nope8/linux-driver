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
#include "ioctl.h"
#include <linux/slab.h>
#include <linux/wait.h> 		//for kfree kmalloc
#include <asm/poll.h> 			// for poll_wait
#include <linux/poll.h> 			// for poll_table
#include <linux/interrupt.h>
#include <plat/irqs.h>

#define GLOBALFIFO_SIZE 0x10 	//全局fifo最大4K字节
#define MEM_CLEAR 0x1  			//清零全局内存
#define GLOBALFIFO_MAJOE 250 	//预设globalfifo的主设备号
MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
dev_t devno;

/*定义设备机构体*/
struct mydevice{
	int len;
	struct cdev cdev;	
};

int irq_ret;

struct mydevice *mydevice;

/*文件打开函数*/
static int dev_open(struct inode *inode, struct file *filp){
	filp->private_data = container_of(inode->i_cdev,struct mydevice,cdev);
	printk(KERN_INFO "mydev opened!\n");
	return 0;
}
/*文件释放函数*/
static int dev_release(struct inode *inode, struct file *filp){
	printk(KERN_INFO "mydev closed!\n");
	return 0;
}

/*文件操作结构体*/
struct file_operations myfops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,
};
/*初始化并添加cdev结构体*/
static void char_setup_cdev(struct mydevice *dev)
{
	int error;
	devno = MKDEV(major,minor);
	cdev_init(&dev->cdev,&myfops);
	/*向内核注册设备号*/
	error = cdev_add(&dev->cdev,devno,1);
	if(error < 0)
	{
		printk(KERN_WARNING "Error:%d adding char dev!\n",error);
	}
	printk(KERN_INFO "char device registed\n");
}

static irqreturn_t irq_handler(int irqno, void *dev_id)
{
	printk("request irqno: %d\n",irqno);
	return IRQ_HANDLED;
}

/*设备驱动模块加载函数*/
static int  __init hello_init(void)
{
	int result;
	devno = MKDEV(major,minor);
	/*申请字符设备驱动区域*/
	if(major)
		result = register_chrdev_region(devno,count,"mydev");
	else{
		/* 动态获得主设备号 */
		result = alloc_chrdev_region(&devno,0,1,"mydev");
		major = MAJOR(devno);
	}
	if(result != 0)
	{
		printk(KERN_WARNING "mydev:can't get major num %d!\n",major);
		return result;
	}
	/* 动态申请设备结构体的内存 */
	mydevice = (struct mydevice *)kmalloc(sizeof(struct mydevice),GFP_KERNEL);
	if(!mydevice){ 			/* 申请失败 */
		result = -ENOMEM;
		printk(KERN_WARNING "fail malloc!\n");
		goto fail_malloc;
	}
	memset(mydevice,0,sizeof(struct mydevice));
	char_setup_cdev(mydevice);
	mydevice->len = 0;

	result = request_irq(IRQ_EINT(1),irq_handler,IRQF_DISABLED|IRQF_TRIGGER_FALLING, "EINT1", NULL);
	result = request_irq(IRQ_EINT(2),irq_handler,IRQF_DISABLED|IRQF_TRIGGER_FALLING, "EINT1", NULL);
	result = request_irq(IRQ_EINT(3),irq_handler,IRQF_DISABLED|IRQF_TRIGGER_FALLING, "EINT1", NULL);
	result = request_irq(IRQ_EINT(4),irq_handler,IRQF_DISABLED|IRQF_TRIGGER_FALLING, "EINT1", NULL);
//	result = request_irq(IRQ_EINT(5),irq_handler,IRQF_DISABLED|IRQF_TRIGGER_FALLING, "EINT1", NULL);
	result = request_irq(IRQ_EINT(6),irq_handler,IRQF_DISABLED|IRQF_TRIGGER_FALLING, "EINT1", NULL);
	result = request_irq(IRQ_EINT(7),irq_handler,IRQF_DISABLED|IRQF_TRIGGER_FALLING, "EINT1", NULL);
	if(result)
		goto out1;
	printk(KERN_INFO "mydev register success!\n");
	
	return 0;
out1:
	cdev_del(&mydevice->cdev); 	/*注销cdev*/
	kfree(mydevice); 	/*释放设备结构体内存*/

fail_malloc: 
	unregister_chrdev_region(devno,count);
	return result;
}
/*模块卸载函数*/
static void __exit hello_cleanup(void)
{
	free_irq(IRQ_EINT(1),NULL);
	free_irq(IRQ_EINT(2),NULL);
	free_irq(IRQ_EINT(3),NULL);
	free_irq(IRQ_EINT(4),NULL);
//	free_irq(IRQ_EINT(5),NULL);
	free_irq(IRQ_EINT(6),NULL);
	free_irq(IRQ_EINT(7),NULL);
	cdev_del(&mydevice->cdev); 	/*注销cdev*/
	kfree(mydevice); 	/*释放设备结构体内存*/

	unregister_chrdev_region(devno,count); /*释放占用的设备号*/
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);
