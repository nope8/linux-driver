/* 
 * 这是一个测试中断的tasklet机制的简单源程序！！
 * 中断三种机制：软中断Softirq、Tasklet、工作队列workqueue。
 * 前两种在下一次任务切换之前执行（SWI），运行于中断上下文，
 * 工作队列则由内核创建专门的线程实现，运行于进程上下文；
 *
 * 测试方法：向创建的字符设备文件输入数据：echo abc > /dev/mydev，
 * 对比不同的jiffies(中断中(line.230)的和中断后的(line.169))；
 * 由于Tasklet是在下一次任务中执行，下一个Tick中执行，所以
 * 中断后jiffies应该大于中断前的jiffies。
 * author：lewis
 * date:2013-7-5
 * email：lxy0371@sina.com
 * 重点知识：
 * 动态获得设备号：函数原型int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,const char *name);
 *
 */
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

#define GLOBALFIFO_SIZE 0x10 	//全局fifo最大4K字节
#define MEM_CLEAR 0x1  			//清零全局内存
#define GLOBALFIFO_MAJOE 250 	//预设globalfifo的主设备号
MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
//char data[128] = "char_read: the message for reading!!!";
dev_t devno;

//spinlock_t *lock;
//struct cdev cdev;
static int dev_fasync(int fd,struct file *filp,int mode); //声明异步处理函数
void dev_do_tasklet(unsigned long arg);


/*定义设备机构体*/
struct mydevice{
	char data[128];
	int len;
	struct cdev cdev;	
	struct semaphore sem;
	wait_queue_head_t rq,wq;
	struct fasync_struct *async_queue;
	struct tasklet_struct tlet;
};

struct mydevice *mydevice;

/*文件打开函数*/
static int dev_open(struct inode *inode, struct file *filp){
	//filp->private_data = mydevice; 		//将结构体mydevice首地址直接赋给filp->private_data

	/*通过struct成员i_cdev得到struct mydevice的指针(首地址)*/
	filp->private_data = container_of(inode->i_cdev,struct mydevice,cdev);
	printk(KERN_INFO "mydev opened!\n");
	return 0;
}
/*文件释放函数*/
static int dev_release(struct inode *inode, struct file *filp){
	dev_fasync(-1,filp,0);
	printk(KERN_INFO "mydev closed!\n");
	return 0;
}
/*读函数*/
static int dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	ssize_t ret = 0;
	struct mydevice *dev = filp->private_data;

	if(count<0) 	return -EINVAL;
#if 0
	printk("read: len = %d",dev->len);
#endif
	down(&dev->sem); 		//获得信号量
	while(dev->len == 0)
	{
		if(filp->f_flags & O_NONBLOCK) 		//判断进程是否为非阻塞I/O
		{
			ret = -EAGAIN; 	//err:Try again
			goto out;
		}
		up(&dev->sem); 		//释放信号量，避免死锁
		/*使进程进入睡眠(条件不成里);否则(条件成立),进程立刻返回，可被信号打断*/
		if(wait_event_interruptible(dev->rq, (dev->len != 0) )) 	
			return -ERESTARTSYS;
		/*产生异步读信号*/
		if(dev->async_queue)
			kill_fasync(&dev->async_queue,SIGIO,POLLOUT);
		down(&dev->sem);
	}

	/*copy内核空间data到用户空间buf*/
	if(count > dev->len) 	count=dev->len;
	if(copy_to_user(buf,dev->data,count))
	{
		ret = -EFAULT;
	}
	else
	{
		memcpy(dev->data, dev->data+count, dev->len-count); 	//data数据前移
		dev->len -= count;
		dev->data[dev->len] = '\0';
		printk(KERN_INFO "read from kernel %d bytes,cur_len:%d,current string:%s\n",count,dev->len,dev->data);
		ret = count;
	}
out:
	up(&dev->sem);
	wake_up_interruptible(&dev->wq);
	return ret;
}
/*写函数*/
static int dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	ssize_t ret = 0;
	struct mydevice *dev = filp->private_data;

	if(count<0) 	return -EINVAL;
	if(count>128) 	return -ENOMEM;

	down(&dev->sem);
	printk(KERN_WARNING "test write or not dev->len:%d\n\n",dev->len);
	while(dev->len == 128)
	{
		printk(KERN_WARNING "\nI'm sleeping1!!\n"); 		//调试语句
		up(&dev->sem);
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		/*让进程进入睡眠状态*/
		if(wait_event_interruptible(dev->wq, (dev->len != 128) ))
			return -ERESTARTSYS;
		/*产生异步写信号*/
		if(dev->async_queue)
			kill_fasync(&dev->async_queue,SIGIO,POLLIN);
		printk(KERN_WARNING "\nI'm sleeping2!!\n"); 		//调试语句
		down(&dev->sem);
	}

	/*copy用户空间buf到内核空间data*/
	if(count > 128 - dev->len )		count = 128 - dev->len;
	if(copy_from_user(dev->data+dev->len, buf, count))
	{
		ret = -EFAULT;
	}
	else
	{
		dev->len += count;
		dev->data[dev->len] = '\0';
		printk(KERN_INFO "receiving from user %d bytes,cur_len:%d current string:%s\n",count,dev->len,dev->data);
		wake_up_interruptible(&dev->rq);
		ret = count;
	}

	/*调度tasklet*/
	tasklet_schedule(&dev->tlet);
	printk(KERN_INFO "write-jiffies:%ld\n",jiffies);
	up(&dev->sem);

	return ret;
} 
/*ioctl设备控制函数*/
static long dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret=0;
	struct mydevice *dev = filp->private_data;

	switch(cmd){
	case DEV_ONE:
		printk(KERN_INFO "one to go");
		break;
	case DEV_TWO:
		printk(KERN_INFO "two to go");
		break;
	case MEM_CLEAR:
		memset(dev->data, 0, sizeof(dev->data));
		dev->len = 0;
		printk(KERN_INFO "data is set to zero");
		break;
	default:
		return -EINVAL;
	}
	return ret;
}
/*非阻塞轮寻函数：poll函数 */
//unsigned int (*poll) (struct file *, struct poll_table_struct *);
static unsigned int dev_poll(struct file *filp,poll_table *wait)
{
	unsigned int mask = 0;
	struct mydevice *dev = filp->private_data;

	down(&dev->sem);
	
	poll_wait(filp,&dev->rq,wait);
	poll_wait(filp,&dev->wq,wait);
	/*fifo非空*/
	if(dev->len != 0)
	{
		mask |= POLLIN | POLLRDNORM; 	//标示数据可获得
	}
	/*fifo非满*/
	if(dev->len != 128)
	{
		mask |= POLLOUT | POLLWRNORM; 	//标示数据可写入
	}
	up(&dev->sem);
	return mask;
}
/*异步通知*/
static int dev_fasync(int fd,struct file *filp,int mode)
{
	struct mydevice *dev = filp->private_data;
	return fasync_helper(fd,filp,mode,&dev->async_queue);
}

void dev_do_tasklet(unsigned long arg)
{
	printk(KERN_INFO "dev_do_tasklet-jiffies:%ld\n",jiffies);
}

/*文件操作结构体*/
struct file_operations myfops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,
	.read = dev_read,
	.write = dev_write,
	.unlocked_ioctl = dev_ioctl,
	.poll = dev_poll
};
/*初始化并添加cdev结构体*/
static void char_setup_cdev(struct mydevice *dev)
{
	int error;
	devno = MKDEV(major,minor);
//	dev->cdev = cdev_alloc();
	cdev_init(&dev->cdev,&myfops);
	/*向内核注册设备号*/
	error = cdev_add(&dev->cdev,devno,1);
	if(error < 0)
	{
		printk(KERN_WARNING "Error:%d adding char dev!\n",error);
	}
	printk(KERN_INFO "char device registed\n");
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
	/*初始化等待队列头*/
	init_waitqueue_head(&mydevice->rq);
	init_waitqueue_head(&mydevice->wq);
	/*初始化信号量*/
	sema_init(&mydevice->sem,1);
	memset(mydevice->data,0,sizeof(mydevice->data));
	mydevice->len = 0;
	tasklet_init(&mydevice->tlet,dev_do_tasklet,(unsigned long)mydevice);
	printk(KERN_INFO "mydev register success!\n");
	
	return 0;

fail_malloc: unregister_chrdev_region(devno,count);
	return result;
}
/*模块卸载函数*/
static void __exit hello_cleanup(void)
{
	devno = MKDEV(major,minor);
	cdev_del(&mydevice->cdev); 	/*注销cdev*/
	kfree(mydevice); 	/*释放设备结构体内存*/

	unregister_chrdev_region(devno,count); /*释放占用的设备号*/
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);
