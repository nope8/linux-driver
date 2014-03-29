/*************************************************  
* File name   : globalmem.c  
* Description : 源程序取自宋宝华书籍  
* Author      : sg131971@qq.com  
* Version     : V1.0  
* Date        :   
* Compiler    : arm-linux-gcc-4.4.3  
* Target      : mini2440(Linux-2.6.32)  
* History     :   
*   <author>  <time>   <version >   <desc>  
    ShiGuang  20100826  增加自动创建设备节点  
    ShiGuang  20100826  增加幻数控制  
*************************************************/   
#include <linux/module.h>    
#include <linux/types.h>    
#include <linux/fs.h>    
#include <linux/errno.h>    
#include <linux/mm.h>    
#include <linux/sched.h>    
#include <linux/init.h>    
#include <linux/cdev.h>    
#include <asm/io.h>    
#include <asm/system.h>    
#include <asm/uaccess.h>    
#include <linux/device.h>       /* device_create() */      
   
/* 取自Ioctl.h (include\asm-generic) */   
//#define _IOC(dir,type,nr,size)  (((dir)  << _IOC_DIRSHIFT) | \    
//                               ((type) << _IOC_TYPESHIFT) | \    
//                               ((nr)   << _IOC_NRSHIFT) | \    
//                               ((size) << _IOC_SIZESHIFT))    
//#define _IOC_TYPECHECK(t) (sizeof(t))    
//#define _IO(type,nr)              _IOC(_IOC_NONE,(type),(nr),0)    
//#define _IOR(type,nr,size)        _IOC(_IOC_READ,(type),(nr),(_IOC_TYPECHECK(size)))    
//#define _IOW(type,nr,size)        _IOC(_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))    
//#define _IOWR(type,nr,size)       _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))    
//#define _IOR_BAD(type,nr,size)    _IOC(_IOC_READ,(type),(nr),sizeof(size))    
//#define _IOW_BAD(type,nr,size)    _IOC(_IOC_WRITE,(type),(nr),sizeof(size))    
//#define _IOWR_BAD(type,nr,size)   _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))    
   
#define GOLBALMEM_MAGIC         'g'    
#define GLOBALMEM_IOC_MAXNR     3    
#define GLOBALMEM_CLEAR         _IO (GLOBALMEM_IOC_MAGIC, 0)      
#define GLOBALMEM_IOCGETDATA    _IOR(GLOBALMEM_IOC_MAGIC, 1, int)      
#define GLOBALMEM_IOCSETDATA    _IOW(GLOBALMEM_IOC_MAGIC, 2, int)     
   
#define GLOBALMEM_SIZE  0x1000  /*全局内存最大4K字节 */    
#define GLOBALMEM_MAJOR 250     /*预设的globalmem的主设备号 */    
   
static struct class *globalmem_class;     
static struct class_device *globalmem_class_dev;    
    
   
static int globalmem_major = 0; /* 采取自动分配方式 */   
   
struct globalmem_dev   
{   
    struct cdev cdev;           /*cdev结构体 */   
    unsigned char mem[GLOBALMEM_SIZE];  /*全局内存 */   
};   
   
struct globalmem_dev *globalmem_devp;   /*设备结构体指针 */   
   
int globalmem_open(struct inode *inode, struct file *filp)   
{   
    /*将设备结构体指针赋值给文件私有数据指针 */   
    filp->private_data = globalmem_devp;   
    return 0;   
}   
   
int globalmem_release(struct inode *inode, struct file *filp)   
{   
    return 0;   
}   
   
static int globalmem_ioctl(struct inode *inodep, struct file *filp, unsigned   
                           int cmd, unsigned long arg)   
{   
    struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针 */   
       
    /* 检测命令的有效性 */     
    if (_IOC_TYPE(cmd) != GLOBALMEM_IOC_MAGIC)     
        return -EINVAL;     
    if (_IOC_NR(cmd) > GLOBALMEM_IOC_MAXNR)     
        return -EINVAL;     
   
    switch (cmd)   
    {   
      case MEM_CLEAR:   
          memset(dev->mem, 0, GLOBALMEM_SIZE);   
          printk(KERN_INFO "globalmem is set to zero\n");   
          break;   
   
      default:   
          return -EINVAL;   
    }   
   
    return 0;   
}   
   
static ssize_t globalmem_read(struct file *filp, char __user * buf, size_t size, loff_t * ppos)   
{   
    unsigned long p = *ppos;   
    unsigned int count = size;   
    int ret = 0;   
    struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针 */   
   
    /*分析和获取有效的写长度 */   
    if (p >= GLOBALMEM_SIZE)   
        return 0;   
    if (count > GLOBALMEM_SIZE - p)   
        count = GLOBALMEM_SIZE - p;   
   
    /*内核空间->用户空间 */   
    if (copy_to_user(buf, (void *)(dev->mem + p), count))   
    {   
        ret = -EFAULT;   
    }   
    else   
    {   
        *ppos += count;   
        ret = count;   
   
        printk(KERN_INFO "read %u bytes(s) from %lu\n", count, p);   
    }   
   
    return ret;   
}   
   
static ssize_t globalmem_write(struct file *filp, const char __user * buf,   
                               size_t size, loff_t * ppos)   
{   
    unsigned long p = *ppos;   
    unsigned int count = size;   
    int ret = 0;   
    struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针 */   
   
    /*分析和获取有效的写长度 */   
    if (p >= GLOBALMEM_SIZE)   
        return 0;   
    if (count > GLOBALMEM_SIZE - p)   
        count = GLOBALMEM_SIZE - p;   
   
    /*用户空间->内核空间 */   
    if (copy_from_user(dev->mem + p, buf, count))   
        ret = -EFAULT;   
    else   
    {   
        *ppos += count;   
        ret = count;   
   
        printk(KERN_INFO "written %u bytes(s) from %lu\n", count, p);   
    }   
   
    return ret;   
}   
   
static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)   
{   
    loff_t ret = 0;   
    switch (orig)   
    {   
      case 0:                  /*相对文件开始位置偏移 */   
          if (offset < 0)   
          {   
              ret = -EINVAL;   
              break;   
          }   
          if ((unsigned int)offset > GLOBALMEM_SIZE)   
          {   
              ret = -EINVAL;   
              break;   
          }   
          filp->f_pos = (unsigned int)offset;   
          ret = filp->f_pos;   
          break;   
      case 1:                  /*相对文件当前位置偏移 */   
          if ((filp->f_pos + offset) > GLOBALMEM_SIZE)   
          {   
              ret = -EINVAL;   
              break;   
          }   
          if ((filp->f_pos + offset) < 0)   
          {   
              ret = -EINVAL;   
              break;   
          }   
          filp->f_pos += offset;   
          ret = filp->f_pos;   
          break;   
      default:   
          ret = -EINVAL;   
          break;   
    }   
    return ret;   
}   
   
static const struct file_operations globalmem_fops = {   
    .owner  = THIS_MODULE,   
    .llseek = globalmem_llseek,   
    .read   = globalmem_read,   
    .write  = globalmem_write,   
    .ioctl  = globalmem_ioctl,   
    .open   = globalmem_open,   
    .release = globalmem_release,   
};   
   
int globalmem_init(void)   
{   
    int result;   
    int err;   
    dev_t devno = MKDEV(globalmem_major, 0);   
   
    /* 申请设备号 */   
    if (globalmem_major)   
        result = register_chrdev_region(devno, 1, "globalmem_proc_devices");   
    else   
    {                           /* 动态申请设备号 */   
        result = alloc_chrdev_region(&devno, 0, 1, "globalmem_proc_devices");   
        globalmem_major = MAJOR(devno);   
    }   
    if (result < 0)   
        return result;   
   
    /* 为新设备分配内存和初始化 */   
    globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);   
    if (!globalmem_devp)   
    {                           /*申请失败 */   
        result = -ENOMEM;   
        goto fail_malloc;   
    }   
    memset(globalmem_devp, 0, sizeof(struct globalmem_dev));   
       
    /* 字符设备的初始化和添加 */   
    cdev_init(&globalmem_devp->cdev, &globalmem_fops);   
    globalmem_devp->cdev.owner = THIS_MODULE;   
    err = cdev_add(&globalmem_devp->cdev, devno, 1);   
    if (err)   
        printk(KERN_NOTICE "Error %d adding globalmem", err);   
       
    /* 自动创建设备节点 */   
    globalmem_class = class_create(THIS_MODULE, "globalmem_sys_class");         
    if (IS_ERR(globalmem_class))     
        return PTR_ERR(globalmem_class);     
       
    globalmem_class_dev = device_create(globalmem_class, NULL, MKDEV(globalmem_major, 0), NULL, "globalmem_dev");        
    if (unlikely(IS_ERR(globalmem_class_dev)))     
        return PTR_ERR(globalmem_class_dev);     
           
    return 0;   
   
  fail_malloc:   
    unregister_chrdev_region(devno, 1);   
    return result;   
}   
   
static void __exit  globalmem_exit(void)   
{   
    cdev_del(&globalmem_devp->cdev);    /*注销cdev */   
    kfree(globalmem_devp);      /*释放设备结构体内存 */   
    unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); /*释放设备号 */   
    device_unregister(globalmem_class_dev);     
    class_destroy(globalmem_class);     
}   
   
     
module_init(globalmem_init);     
module_exit(globalmem_exit);     
     
MODULE_AUTHOR("WHUT-ShiGuang");     
MODULE_DESCRIPTION("Mini2440 Globalmem Driver");     
MODULE_VERSION("1.0");     
MODULE_LICENSE("GPL"); 
