
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define DRIVER_AUTHOR "lewis"
#define DRIVER_DESC 	"A driver sample"
#define SN1 	2002
#define SN2 	2013

/**parameter**/
short myshort =11;
int myint = 22;
long mylong = 33;
char *mystring = "My name is lewis!";
static int myarray[] = {0,1,2,3};
static int argc = 4;


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("s5pc100");
MODULE_INFO(tag,"Test");

module_param(myshort,short,0700);
MODULE_PARM_DESC(myshort,"a short integer");

module_param(myint,int,0000);
MODULE_PARM_DESC(myint,"a integer");

module_param(mylong,long,0400);
MODULE_PARM_DESC(mylong,"a long integer");

module_param(mystring,charp,0400);
MODULE_PARM_DESC(mystring,"a character string");

module_param_array(myarray,int, &argc,0774);
MODULE_PARM_DESC(myarray,"an integer array");


static int  __init hello_init(void)
{
	printk(KERN_INFO "hello world!\n");
	return 0;
}

static void __exit hello_cleanup(void)
{
	printk(KERN_INFO "goodbye world!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);

