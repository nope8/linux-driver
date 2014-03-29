/*
 * 这是一个基于platform的adc测试程序
 * author:lewis
 * date:2013-7-11
 * 测试方法：S5PC100平台下，加载生成的ko文件，运行test程序，不停的打印出电压值。
 * 主要函数：
 * 	s5pc100_adc_probe():当驱动名字和设备名字匹配时，probe被调用,完成
 * 		申请资源，初始化、注册设备，获得并打开adc设备时钟,为功能操作做准备。
 * 	adc_read():设置adc模式，从adc中读取Y坐标的值，copy到用户空间。
 *  	
 * 中断申请函数：request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev)
 *  __devexit修饰的的函数：表示函数支持hotplug；
 * 用到的结构体：
 * struct resource {
 *		resource_size_t start;
 *		resource_size_t end;
 *		const char *name;
 *		unsigned long flags;
 *		struct resource *parent, *sibling, *child;
 *	};
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
#include <asm/system.h>
#include <linux/delay.h>
#include <linux/slab.h> 		//for kfree kmalloc
#include <linux/wait.h> 		
#include <mach/regs-gpio.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#define GLOBALFIFO_SIZE 0x10 	//全局fifo最大4K字节
#define MEM_CLEAR 0x1  			//清零全局内存
#define GLOBALFIFO_MAJOE 250 	//预设globalfifo的主设备号

#define S5PC100_ADCDAT0 0xc 	
#define S5PC100_ADCCON 	0x0
#define S5PC100_ADCMUX 	0x1c 	//用于频道选择
#define S5PC100_ADCCLRINT 0x18 	//清中断

MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
dev_t devno;

void __iomem *adc_base;
wait_queue_head_t readq;
unsigned long flags = 0;
struct clk *clk;
int ret;
/*定义设备机构体*/
struct myadc{
	struct cdev cdev;
}myadc;

struct resource *res_mem,*res_irq;

irqreturn_t adc_irq_handler(int irqno,void *devid)
{
	flags = 1;

	wake_up_interruptible(&readq);
	writel(0, adc_base+S5PC100_ADCCLRINT);

	return IRQ_HANDLED;
}

static int adc_open(struct inode *inode, struct file *filp){
	printk(KERN_INFO "myadc opened!\n");
	return 0;
}


static int adc_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "myadc closed!\n");
	return 0;
}

static int adc_read(struct file * filp, char __user *buf, size_t count, loff_t *offset)
{
	int data;

	writel(0, adc_base+S5PC100_ADCMUX); 	//选择频道0
	/* 设置12bitA/D转换模式，预分频值：255，打开A/D */
	writel(1<<16 | 1<<14 | 0xff<<6 | 1<<0,adc_base+S5PC100_ADCCON);

	if(wait_event_interruptible(readq, flags!=0))
		return -ERESTARTSYS;
	/* 从ADCDAT0中读取Y-Position Conversion data value */
	data = readl(adc_base+S5PC100_ADCDAT0) & 0xfff; 	
	
	if(copy_to_user(buf,(char *)&data,sizeof(data)))
		return -EFAULT;
	
	flags = 0;

	return sizeof(data);
}

/*文件操作结构体*/
struct file_operations myfops = {
	.owner = THIS_MODULE,
	.open = adc_open,
	.release = adc_release,
	.read = adc_read
};

/*当驱动名字和设备名字匹配时，probe被调用*/
static int s5pc100_adc_probe(struct platform_device *pdev)
{

	init_waitqueue_head(&readq); 	//务必初始化等待队列
	
	/* 申请内存资源、中断资源(号) */
	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	res_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 1);

	if((res_mem == NULL) || (res_irq == NULL))
		return -ENODEV;
	printk("mem start = 0x%x,end = 0x%x\n",res_mem->start,res_mem->end);
	printk("irqno = %d\n",res_irq->start);
	
	adc_base = ioremap(res_mem->start,res_mem->end - res_mem->start);
	if(adc_base == NULL)
		return -ENOMEM;
	/* 申请中断 */
	ret = request_irq(res_irq->start,adc_irq_handler,flags,"adc",NULL);
	if(ret < 0)
		goto err1;

	if(register_chrdev_region(devno,count,"adc") < 0)
		goto err2;
	/* 初始化cdev ，即关联cdev和file_operations*/
	cdev_init(&myadc.cdev,&myfops);
	ret = cdev_add(&myadc.cdev,devno,1);
	if(ret < 0)
	{
		printk(KERN_WARNING "Error: %d fail adding adc device\n",ret);
		goto err3;
	}

	clk = clk_get(NULL,"adc"); 		//获得adc时钟控制结构体
	if(clk == NULL)
	{
		ret = -ENODEV;
		goto err4;
	}
	clk_enable(clk); 				//打开adc时钟
	printk(KERN_INFO "platform:match ok !\n");
	return 0;
err4:
	cdev_del(&myadc.cdev);
err3:
	unregister_chrdev_region(devno,count);
err2:
	free_irq(res_irq->start,NULL);
err1:
	iounmap(adc_base);
	return ret;
}

static int __devexit s5pc100_adc_remove(struct platform_device *pdev)
{
	clk_disable(clk);
	clk_put(clk);

	cdev_del(&myadc.cdev);
	unregister_chrdev_region(devno,count);

	iounmap(adc_base);
	free_irq(res_irq->start, NULL);
	
	printk(KERN_INFO "adc driver unregistered");

	return 0;
}
struct platform_driver adc_driver = {
	.probe = s5pc100_adc_probe,
	.remove = __devexit_p(s5pc100_adc_remove),
	.driver = {
		.name = "s5pc100-adc",
	}
};

static int __init s5pc100_adc_init(void)
{
	devno = MKDEV(major,minor);
	/**/
	platform_driver_register(&adc_driver); 		//注册adc驱动
	printk(KERN_INFO "module load!\n");
	return 0;
}

static void __exit s5pc100_adc_cleanup(void)
{
	platform_driver_unregister(&adc_driver); 	//注销adc驱动
	printk(KERN_INFO "module unload!\n"); 		
}

module_init (s5pc100_adc_init);
module_exit (s5pc100_adc_cleanup);

