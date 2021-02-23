#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>

#include <linux/cdev.h> //cdev
#include <linux/device.h> //udev
#include <linux/sched.h> //schedule
#include <linux/uaccess.h> //copy_to_user
#include <linux/slab.h> //kmalloc()

#define DEVICE_NAME "second_drv"
static int second_major=0; /*主设备号*/
struct class *second_class;
int ret;

/* second设备结构体 */
static struct second_dev
{
    struct cdev cdev;/*cdev 结构体*/
    atomic_t counter;/*一共经历了多少时间；定义原子变量*/    
    struct timer_list s_timer;/*定义一个定时器*/
};

/* 定义设备结构体指针 */
struct second_dev *second_devp;

/* 定时器处理函数 */
static void second_timer_handler(unsigned long arg)
{
    mod_timer(&second_devp->s_timer,jiffies+HZ); /*再次调度定时器*/
    atomic_inc(&second_devp->counter);  /*counter 加1*/
    printk(KERN_NOTICE"current jiffies is %d!!\n",jiffies);
}

/* 打开设备 */
static int second_open(struct inode *inode, struct file *filp)
{
    //printk("in the second_open!!\n");
    filp->private_data=second_devp; //将设备结构体指针赋值给文件私有数据指针

    init_timer(&second_devp->s_timer);//初始化定时器
    second_devp->s_timer.function=&second_timer_handler;
    second_devp->s_timer.expires=jiffies+HZ;
    add_timer(&second_devp->s_timer); //添加（注册）定时器，加入到内核定时器链表中

    atomic_set(&second_devp->counter,0);//计数清零
    
    return 0;
}

/* 关闭设备 */
static int second_release(struct inode *inode,struct file *filp)
{
    del_timer(&second_devp->s_timer);//删除定时器
    return 0;
}

/* 读设备 */
static ssize_t second_read(struct file *filp,char __user *buf,ssize_t count,loff_t *ppos)
{    
    //printk("in the second_read!!\n");
    int counter;
    counter=atomic_read(&second_devp->counter); //获取count值

    if(copy_to_user(buf,&counter,count))
        return -EFAULT;
    else 
        return sizeof(unsigned int);
}

/* 写设备 */
static ssize_t second_write(struct file *filp,const char __user *buf,ssize_t count,loff_t *ppos)
{
    return 0;
}

/* 文件操作结构体 */
static const struct file_operations second_fops={
    .owner = THIS_MODULE,
    .open  = second_open,
    .release = second_release,
    .read  = second_read,
    .write = second_write,
};

/* 初始化并注册cdev */
static void second_setup_cdev(struct second_dev *dev,int index)
{
    int err;
    dev_t devno=MKDEV(second_major,index);
    cdev_init(&dev->cdev,&second_fops);//初始化cdev成员
    dev->cdev.owner=THIS_MODULE;
    err=cdev_add(&dev->cdev,devno,1);//向系统注册字符设备
    if(err)
        printk(KERN_NOTICE"error=%d",err);
}

/*设备驱动模块加载函数*/
static int __init second_init(void)
{
    printk("--------------hello second_dev---------------- \n");
    /*申请设备号*/
    //int ret;
    dev_t devno=MKDEV(second_major,0);/*获得设备号*/
    if(second_major)
        ret=register_chrdev_region(devno,1,DEVICE_NAME);
    else
        {    
            /*动态申请设备号*/
            ret=alloc_chrdev_region(&devno,0,1,DEVICE_NAME);
            second_major=MAJOR(devno);
        }
    if(ret)
        {
            printk("request chrdev failed!!\n");
            return ret;
        }

    /*动态申请设备结构体内存*/
    second_devp=kmalloc(sizeof(struct second_dev),GFP_KERNEL);
    if(!second_devp)
        {
            ret= -ENOMEM;
            goto fail_kmalloc;
        }
    memset(second_devp,0,sizeof(struct second_dev));/*申请到的内存空间清零*/

    /*注册字符设备*/
    second_setup_cdev(second_devp,0);

    /*用udev机制自动创建设备文件结点*/
    second_class=class_create(THIS_MODULE,"second_class");/*在sys/class下添加second_class这个类*/
    if (IS_ERR(second_class)) 
    {
        printk(KERN_ERR "class_create() failed for second_class\n");
        goto fail_class_create;
    }
    device_create(second_class,NULL,devno,NULL,DEVICE_NAME); /*创建设备/dev/$DEVICE_NAME*/

    printk("init second_drv success!!,major=%d!!\n",second_major);
    return 0;
    fail_class_create:
        cdev_del(&second_devp->cdev);/*注销cdev*/
        kfree(second_devp);/*释放设备结构体内存*/
    fail_kmalloc:
        unregister_chrdev_region(devno,1);/*释放设备号*/
        
    return ret;
}


/*设备驱动模块卸载函数*/
static void __exit second_exit(void)
{
    printk("--------------goodbye second_dev---------------- \n");
    device_destroy(second_class,MKDEV(second_major,0));/*注销设备*/
    class_destroy(second_class);/*注销类*/
    cdev_del(&second_devp->cdev);/*注销cdev*/
    kfree(second_devp);/*释放设备结构体内存*/
    unregister_chrdev_region(MKDEV(second_major,0),1);/*释放设备号*/
}


MODULE_LICENSE("GPL");
module_init(second_init);
module_exit(second_exit);