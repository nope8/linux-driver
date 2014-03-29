#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h> 		//for kfree kmalloc kzalloc
#include <linux/wait.h> 		
#include <asm/poll.h> 			// for poll_wait
#include <linux/poll.h> 		// for poll_table
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mutex.h>

#define CMD_READ_ID 	 0x9f
#define CMD_WRITE_ENABLE 0x06
#define CMD_BULK_ERASE   0xc7
#define CMD_PAGE_PROGRAM 0x02
#define CMD_READ_BYTE  	 0x03
#define CMD_RDSR 		 0x05

#define FLASH_PAGE_SIZE  256

/*statu resgister bits*/
#define SR_WIP 	1
#define SR_WEL 	2
/*ID NUMBERS*/
#define MANUFACTURER_ID  0x20
#define DEVICE_ID 		 0x1120
/* Define max times to check status register before we give up. */
#define MAX_READY_WAIT_COUNT    100000
#define CMD_SZ 4
MODULE_LICENSE("GPL");

int major=250,minor=0;
int count = 1;
int flag = 1;
dev_t devno;

/*定义设备机构体*/
struct m25p10a{
	struct spi_device *spi;
	struct mutex lock;
	char erase_opcode;
	char cmd[CMD_SZ];
};

static int read_sr(struct m25p10a *flash)
{
	ssize_t retval;
	u8 code = CMD_RDSR;
	u8 val;

	retval = spi_write_then_read(flash->spi, &code, 1, &val, 1);

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return retval;
}

static int wait_till_ready(struct m25p10a *flash)
{
	int count;
	int sr;

	for(count=0; count<MAX_READY_WAIT_COUNT; count++)
	{
		if((sr = read_sr(flash)) < 0)
			break;
		else if(!(sr & SR_WIP))
			return 0;
	}
	printk("in (%s): count = %d\n",__func__,count);

	return 1;
}

static inline int write_enable(struct m25p10a *flash)
{
	flash->cmd[0] = CMD_WRITE_ENABLE;
	return spi_write(flash->spi,flash->cmd,1);
}

static int check_id(struct m25p10a *flash)
{
	char buf[10] = {0};

	flash->cmd[0] = CMD_READ_ID;

	spi_write_then_read(flash->spi,flash->cmd,1,buf,3);
	printk("Manufacture ID:0x%x\n",buf[0]);

	return 0;
}

static int erase_chip(struct m25p10a *flash)
{
	if(wait_till_ready(flash))
		return -1;

	flash->cmd[0] = CMD_WRITE_ENABLE;
	if(spi_write(flash->spi,flash->cmd,1))
		return -1;

	flash->cmd[0] = CMD_BULK_ERASE;
	if(spi_write(flash->spi,flash->cmd,1))
		return -1;

	return 0;
}

static int m25p10a_write(struct m25p10a *flash,loff_t to, size_t len,const char *buf)
{
	struct spi_transfer st[2];
	struct spi_message msg;

	if(wait_till_ready(flash))
		return -1;

	flash->cmd[0] = CMD_WRITE_ENABLE;
	if(spi_write(flash->spi,flash->cmd,1))
		return -1;

	spi_message_init(&msg);
	memset(st,0,sizeof(st)*2);
	if(len > 256)
		return -1;
	to = to - to%256;

	flash->cmd[0] = CMD_PAGE_PROGRAM;
	flash->cmd[1] = to>>16;
	flash->cmd[2] = to>>8;
	flash->cmd[3] = to;

	st[0].tx_buf = flash->cmd;
	st[0].len = CMD_SZ;
	spi_message_add_tail(&st[0],&msg);

	st[1].tx_buf = buf;
	st[1].len = len;
	spi_message_add_tail(&st[1],&msg);

	spi_sync(flash->spi,&msg);
	
	return 0;
}

static int m25p10a_read(struct m25p10a *flash,loff_t from,size_t len,const char *buf)
{
	int r_count=0;
	int i;

#if 0
	flash->cmd[0] = CMD_READ_BYTE;
	flash->cmd[1] = from>>16;
	flash->cmd[2] = from>>8;
	flash->cmd[3] = from;
	spi_write_then_read( flash->spi, flash->cmd, CMD_SZ, buf, 20 	);

	for( i = 0; i < 20; i++)
	{
		printk("%x\n", buf[i]);
	}
#endif

#if 1
	struct spi_transfer st[2];
	struct spi_message msg;

	if(wait_till_ready(flash))
		return -1;

	spi_message_init(&msg);
	memset(st,0,sizeof(st)*2);

	flash->cmd[0] = CMD_READ_BYTE;
	flash->cmd[1] = from>>16;
	flash->cmd[2] = from>>8;
	flash->cmd[3] = from;

	st[0].tx_buf = flash->cmd;
	st[0].len = CMD_SZ;
	spi_message_add_tail(&st[0],&msg);

	st[1].tx_buf = buf;
	st[1].len = len;
	spi_message_add_tail(&st[1],&msg);
	
	mutex_lock(&flash->lock);
	if(wait_till_ready(flash))
	{
		mutex_unlock(&flash->lock);
		return -1;
	}

	spi_sync(flash->spi,&msg);
	r_count = msg.actual_length - CMD_SZ;
	printk( "in (%s): read %d bytes\n", __func__, r_count);
	
	for( i = 0; i < r_count; i++ ) {
		printk("0x%02x\n", buf[i]);
	}
	mutex_unlock(&flash->lock);
#endif
	return 0;	
}
static int m25p10a_probe(struct spi_device *spi)
{
	int ret = 0;
	struct m25p10a *flash;
	char buf[256];

	flash = kzalloc(sizeof(struct m25p10a),GFP_KERNEL);
	flash->spi = spi;
	mutex_init(&flash->lock);

	spi_set_drvdata(spi,flash);
	check_id(flash);
	ret = erase_chip(flash);

	memset(buf,0x7,256);
	m25p10a_write(flash,0,20,buf);

	memset(buf,0,256);
	m25p10a_read(flash,0,25,buf);

	return 0;
}

static int m25p10a_remove(struct spi_device *spi)
{
	return 0;
}

static struct spi_driver m25p10a_driver = {
	.probe = m25p10a_probe,
	.remove = m25p10a_remove,
	.driver = {
		.name = "m25p10a",
	},
};

/*设备驱动模块加载函数*/
static int  __init hello_init(void)
{
	printk(KERN_INFO "spi driver load!\n");
	spi_register_driver(&m25p10a_driver);
	return 0;
}
/*模块卸载函数*/
static void __exit hello_cleanup(void)
{
	spi_unregister_driver(&m25p10a_driver);
	printk(KERN_INFO "unregister mydev!\n");
}

module_init (hello_init);
module_exit (hello_cleanup);

