#include <linux/sched.h>
#include <linux/kernel.h>/* printkntk() */
#include <linux/slab.h>/* kmalloc() */
#include <linux/fs.h>/* everything... */
#include <linux/errno.h>/* error codes */
#include <linux/timer.h>
#include <linux/types.h>/* size_t */
#include <linux/fcntl.h>/* O_ACCMODE */
#include <linux/hdreg.h>/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/spinlock.h>

#define GLOBALFIFO_SIZE 0x10 	//全局fifo最大4K字节
#define MEM_CLEAR 0x1  			//清零全局内存
#define GLOBALFIFO_MAJOE 250 	//预设globalfifo的主设备号


MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
dev_t devno;

struct gendisk *virtdisk;
struct request_queue *queue;
spinlock_t lock;
unsigned char *virt_buf;

const struct block_device_operations virt_fops = {
	.owner = THIS_MODULE,
};

static void do_queue(struct request_queue *q)
{
	struct request *req;

	while((req = blk_fetch_request(q)) != NULL)
	{
		while(1)
		{
			if(req->cmd_flags & 1)
			{
				memcpy(virt_buf+req->__sector*512,req->buffer,blk_rq_cur_sectors(req)*512);
				printk(KERN_ERR "disk write\n");
			}
			else
			{
				memcpy(req->buffer,virt_buf+req->__sector*512,blk_rq_cur_sectors(req)*512);
				printk(KERN_ERR "disk read\n");
			}

			if(!__blk_end_request_cur(req,0))
				break;
		}
	}
}

static int virt_init(void)
{
	spin_lock_init(&lock);

	virt_buf = kzalloc(1024*1024, GFP_KERNEL);
	if(virt_buf == NULL)
		return -1;
	major = register_blkdev(0,"virt_disk");

	virtdisk = alloc_disk(16);

	queue = blk_init_queue(do_queue,&lock);
	virtdisk->queue = queue;
	virtdisk->major = major;
	strcpy(virtdisk->disk_name,"virtdisk");
	virtdisk->first_minor = 0;
	virtdisk->fops = &virt_fops;
	set_capacity(virtdisk,1024*1024/512);
	add_disk(virtdisk);

	return 0;
}


/*模块卸载函数*/
static void __exit virt_cleanup(void)
{
	del_gendisk(virtdisk);
	put_disk(virtdisk);
	blk_cleanup_queue(queue);
	unregister_blkdev(0,"virt_disk");
	printk(KERN_INFO "unload ramdisk!\n");
}

module_init (virt_init);
module_exit (virt_cleanup);

