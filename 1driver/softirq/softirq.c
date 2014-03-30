#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>			//for macro KERN_ERR KERN_INFO
#include <linux/interrupt.h>
#include <linux/sched.h>		//for jiffies

MODULE_LICENSE("GPL");
int irq;
char *interface;

module_param(irq, int, 0644);
module_param(interface, charp, 0644);

static irqreturn_t do_softirq_func(int irq, void *dev)
{
	static int count = 1;
	printk(KERN_INFO "irq[%d]interrupt at %ld\n", irq, jiffies);
	count ++;

	return 0;
}

static int __init dev_init(void)
{
	int result = 0;
	result = request_irq(irq, do_softirq_func, IRQF_SHARED, interface, (void *)&irq);
	printk(KERN_ERR "result:%d\n", result);
	if(result)
	{
		printk(KERN_ERR "request_irq error:%d.\n", result);
		return result;
	}

	printk(KERN_INFO "interface:%s, irq:%d\n", interface, irq);
	printk(KERN_INFO "register irq success!\n");

	return 0;
}


static void __exit dev_exit(void)
{
	printk(KERN_INFO "unregister irq success!\n");
}


module_init(dev_init);
module_exit(dev_exit);
