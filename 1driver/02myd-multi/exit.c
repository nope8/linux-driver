#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

static void __exit hello_cleanup(void)
{
	printk(KERN_INFO "final exit!\n");
}

module_exit (hello_cleanup);

