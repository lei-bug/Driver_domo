#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
static int major = 199;
static int minor = 0;
static dev_t devno;
static struct cdev cdev;//使用cdev结构体来描述一个字符设备

/* open */
static int hello_open (struct inode *inode, struct file *filep)
{
	printk("hello_open \n");
	return 0;
}

/* 设备文件操作集合 */
static struct file_operations hello_ops=
{
	.open = hello_open,			
};
 
/* 加载函数 */
static int hello_init(void)
{
	int ret;	
	printk("hello_init\n");
	devno = MKDEV(major,minor);
    /* 通过major和minor构建设备号，只是构建设备号。
    并未注册，需要调用 register_chrdev_region 静态申请；*/
	ret = register_chrdev_region(devno, 1, "hello");//注册设备
	if(ret < 0)
	{
		printk("register_chrdev_region fail \n");
		return ret;
	}
	cdev_init(&cdev,&hello_ops);//建立cdev与 file_operations之间的连接
	ret = cdev_add(&cdev,devno,1);//向系统添加一个cdev以完成注册;
	if(ret < 0)
	{
		printk("cdev_add fail \n");
		return ret;
	}	
	return 0;
}

/* 卸载函数 */
static void hello_exit(void)
{
	cdev_del(&cdev);//注销cdev
	unregister_chrdev_region(devno,1);//释放设备号
	printk("hello_exit \n");
}


MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);