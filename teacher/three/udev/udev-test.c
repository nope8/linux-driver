/*
 * Copyright (C) 2005 Farsight
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/device.h>


MODULE_LICENSE ("GPL");

int hello_major = 250;
int hello_minor = 0;
int number_of_devices = 1;

struct cdev cdev;
dev_t dev = 0;

struct file_operations hello_fops = {
	.owner = THIS_MODULE,
};

struct class *my_class;


static void char_reg_setup_cdev (void)
{
	int error, devno = MKDEV (hello_major, hello_minor);
	cdev_init (&cdev, &hello_fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &hello_fops;
	error = cdev_add (&cdev, devno , 1);
	if (error)
		printk (KERN_NOTICE "Error %d adding char_reg_setup_cdev", error);

	/* creating your own class */
	my_class =class_create(THIS_MODULE, "farsight_class");
	if(IS_ERR(my_class)) {
		printk("Err: failed in creating class.\n");
		return ;
	}

	/* register your own device in sysfs, and this will cause udevd to create corresponding device node */
	//class_device_create(my_class,NULL, devno, NULL,"farsight_dev");//add by lht
	device_create(my_class,NULL, devno, NULL,"farsight_dev");
	device_create(my_class,NULL, devno+1, NULL,"farsight_dev1");
}

static int __init hello_2_init (void)
{
	int result;

	dev = MKDEV (hello_major, hello_minor);
	result = register_chrdev_region (dev, number_of_devices, "test");

	if (result<0) {
		printk (KERN_WARNING "hello: can't get major number %d\n", hello_major);
		return result;
	}

	char_reg_setup_cdev ();
	printk (KERN_INFO "char device registered\n");
	return 0;
}

static void __exit hello_2_exit (void)
{
	dev_t devno = MKDEV (hello_major, hello_minor);

	cdev_del (&cdev);

	//class_device_destroy(my_class, devno);
	//class_destroy(my_class);

	unregister_chrdev_region (devno, number_of_devices);

	device_destroy(my_class, devno);
	device_destroy(my_class, devno+1);
	class_destroy(my_class);


}

module_init (hello_2_init);
module_exit (hello_2_exit);
