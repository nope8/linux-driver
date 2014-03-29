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
#include <asm/poll.h> 			// for poll_wait
#include <linux/poll.h> 			// for poll_table
#include <linux/interrupt.h>
#include <plat/irqs.h>
#include "ioctl.h"
#include <linux/clk.h>
#include <linux/i2c.h>

#define GLOBALFIFO_SIZE 0x10 	//全局fifo最大4K字节
#define MEM_CLEAR 0x1  			//清零全局内存
#define GLOBALFIFO_MAJOE 250 	//预设globalfifo的主设备号

#define S5PC100_PA_WTCON 0xea200000
#define S5PC100_PA_WTDAT 0xea200004
#define S5PC100_PA_WTCNT 0xea200008

struct clk *clk;
struct i2c_client *new_client;

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

struct mydevice *mydevice;

/*文件打开函数*/
static int dev_open(struct inode *inode, struct file *filp){
	printk(KERN_INFO "mydev opened!\n");
	return 0;
}
/*文件释放函数*/
static int dev_release(struct inode *inode, struct file *filp){	cdev_del(&mydevice->cdev); 	/*注销cdev*/

	printk(KERN_INFO "mydev closed!\n");
	return 0;
}

static int read_value(struct i2c_client *client)
{
	char buf1[2];
	char buf2[2];
	struct i2c_msg msg[2];
	int status;	
	
	//@len: numbers of data bytes readfrom or writtento i2c slave address
	//@addr:i2c slave address
	//@flags:control read or write  
	msg[0].len = 1;  			
	msg[0].addr = client->addr;
	msg[0].flags = 0; 		//write  @=I2C_M_RD
	msg[0].buf = buf1; 
	msg[0].buf[0] = 0x0;
	msg[1].len = 2;
	msg[1].addr = client->addr;
	msg[1].flags = 1; 		//read
	msg[1].buf = buf2;

//int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
	status = i2c_transfer(client->adapter,msg,2);
	if(status < 0)
		return -EFAULT;

	return (buf2[0]<<8) | buf2[1] ;
}

static int dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	int status;
	
	status = read_value(new_client);
	if(copy_to_user(buf,&status,sizeof(status)))
		return -EFAULT;

	return sizeof(status);

}

/*文件操作结构体*/
struct file_operations myfops = {
	.owner = THIS_MODULE,
	.read = dev_read,
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
	printk(KERN_INFO "lm75 registered!\n");
}

static int lm75_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int result,count = 1;

	new_client = client;

	printk(KERN_INFO "test0 line:%d\n",__LINE__);

	if(!(count = i2c_check_functionality(client->adapter,I2C_FUNC_SMBUS_BYTE_DATA)))
	{
		printk(KERN_INFO "test0 count:%d\n",count);
		return -EIO;
	}

	devno = MKDEV(major,minor);
	/*申请字符设备驱动区域*/
	if(major)
		result = register_chrdev_region(devno,count,"lm75");
	else{
		/* 动态获得主设备号 */
		result = alloc_chrdev_region(&devno,0,1,"lm75");
		major = MAJOR(devno);
	}
	printk(KERN_INFO "test2");
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

fail_malloc: 
	unregister_chrdev_region(devno,count);
	return result;
}

static int lm75_remove(struct i2c_client *client)
{
	kfree(mydevice); 	/*释放设备结构体内存*/
	unregister_chrdev_region(devno,count); /*释放占用的设备号*/
	printk(KERN_INFO "lm75 driver unload!\n");
	return 0;
}

static const struct i2c_device_id lm75_ids[] = {
	{"lm75",0},
	{"lm75a",1},
};

static struct i2c_driver lm75_driver = {
	.driver = {
		.name = "lm75",
	},
	.probe = lm75_probe,
	.remove = lm75_remove,
	.id_table = lm75_ids,
};

/*设备驱动模块加载函数*/
static int  __init hello_init(void)
{
	printk(KERN_INFO "lm75 driver load!\n");
		
	return i2c_add_driver(&lm75_driver);
}

/*模块卸载函数*/
static void __exit hello_cleanup(void)
{
	i2c_del_driver(&lm75_driver);
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);

