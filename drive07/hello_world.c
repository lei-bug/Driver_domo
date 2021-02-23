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


/* hello_world设备结构体 */
static struct hello_world_dev
{
    struct cdev cdev;/*cdev 结构体*/
    atomic_t counter;/*一共经历了多少时间；定义原子变量*/    
    struct timer_list s_timer;/*定义一个定时器*/
};


/* 定义设备结构体指针 */
struct hello_world_dev *hello_world_devp;


/* 定时器处理函数 */
static void hello_world_timer_handler(unsigned long arg)
{
    mod_timer(&hello_world_devp->s_timer,jiffies+HZ); /*再次调度定时器*/
    atomic_inc(&hello_world_devp->counter);  /*counter 加1*/
    printk(KERN_NOTICE "current jiffies is %ld!!\n",jiffies);
}

/* 打开设备 */
static int hello_world_open(struct inode *inode, struct file *filp)
{
    //printk("in the second_open!!\n");

    init_timer(&hello_world_devp->s_timer);//初始化定时器
    hello_world_devp->s_timer.function=&hello_world_timer_handler;//初始化定时器处理函数
    hello_world_devp->s_timer.expires=jiffies+HZ;//设置超时时间一秒
    add_timer(&hello_world_devp->s_timer); //添加（注册）定时器，加入到内核定时器链表中

    atomic_set(&hello_world_devp->counter,0);//计数清零
    
    return 0;
}

//释放设备
static int hello_world_release(struct inode *inode, struct file *filp){
    del_timer(&hello_world_devp->s_timer);//删除定时器
    //printk(KERN_DEBUG "close");
    return 0;
}

/* 文件操作结构体 */
static const struct file_operations hello_world_fops={
    .owner = THIS_MODULE,
    .open  = hello_world_open,
    .release = hello_world_release,
};

/* 初始化并注册cdev */
static void hello_world_setup_cdev(struct hello_world_dev *dev,int index)
{
    int ret;
    dev_t devno = MKDEV(201,index);//将主、次设备号转换成dev_t类型
    cdev_init(&dev->cdev,&hello_world_fops);//关联file_operations
    dev->cdev.owner=THIS_MODULE;
    ret = cdev_add(&dev->cdev,devno,1);//向系统注册一个字符设备
        if(ret < 0){
        printk(KERN_NOTICE "error");
    }

    /*
     ret = register_chrdev(201, "helloworld",&hello_world_fops);//注册设备
        if(ret < 0)
        {
                printk("register_chrdev fail \n");
        }*/
        
}

/* 入口函数 */
static int __init hello_world_init(void)
{
    printk(KERN_DEBUG "---------hello world!!!---------\n");


/*动态申请设备结构体内存*/
    hello_world_devp=kmalloc(sizeof(struct hello_world_dev),GFP_KERNEL);
    if(!hello_world_devp)
        {
            printk(KERN_DEBUG " fialed\n");
            return -1;
        }
    memset(hello_world_devp,0,sizeof(struct hello_world_dev));/*申请到的内存空间清零*/

/* 调用函数 */
    hello_world_setup_cdev(hello_world_devp,0);

    return 0;

}

/* 出口函数 */
static void __exit hello_world_exit(void)
{
    unregister_chrdev(201, "helloworld");
    printk(KERN_DEBUG "---------goodbye world!!!---------\n");
    kfree(hello_world_devp);/*释放设备结构体内存*/

}

MODULE_LICENSE("GPL"); 
module_init(hello_world_init);
module_exit(hello_world_exit);