//头文件
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include <asm/io.h>
#include <asm/string.h>
#include <asm/uaccess.h>
#include <asm-generic/ioctl.h>

#define LED_NUM_ON        _IOW('L',0x1122,int)
#define LED_NUM_OFF        _IOW('L',0x3344,int)
#define LED_ALL_ON        _IO('L',0x1234)
#define LED_ALL_OFF        _IO('L',0x5678)

//定义一个按键的数据包
struct button_event{
    int code;         //按键的名称---键值:KEY_DOWN
    int value;        //按键的状态---按下:1,松开:0
};


//面向对象编程----设计设备的类型
struct s5pv210_button{
    //unsigned int major;
    dev_t  devno;
    struct class * cls;
    struct device * dev;
    struct cdev  *cdev;
    unsigned int irqno;
    struct button_event event;
    int data;
};
struct s5pv210_button *button_dev;

/* 实现中断处理函数--------当触发中断时会被执行 */
irqreturn_t button_irq_svc(int irqno, void *dev)
{
    int value;
    printk("--------^_^ %s------------\n",__FUNCTION__);
    //获取按键信息
    value = gpio_get_value(S5PV210_GPH0(1));
    //判断是按下还是松开
    if(value){
        //松开
        printk("kenel:keydown up!\n");
        button_dev->event.code = KEY_DOWN;
        button_dev->event.value = 0;
    }else{
        //按下
        printk("kenel:keydown pressed!\n");
        button_dev->event.code = KEY_DOWN;
        button_dev->event.value = 1;
    }

    return IRQ_HANDLED;
}

/* 实现设备操作接口 */
int button_open(struct inode *inode, struct file *filp)
{

    printk("--------^_^ %s------------\n",__FUNCTION__);
    
    return 0;
}
ssize_t button_read(struct file *filp , char __user *buf , size_t size, loff_t *flags)
{
    int ret;
    printk("--------^_^ %s------------\n",__FUNCTION__);
    /* 将内核数据转换为用户空间数据 */
    ret = copy_to_user(buf,&button_dev->event,size);
    if(ret > 0){
        printk("copy_to_user error!\n");
        return -EFAULT;
    }

    /* 将数据返回给应用空间后，清空数据包 */
    memset(&button_dev->event,0,sizeof(button_dev->event));
    return size;
}

ssize_t button_write(struct file *filp, const char __user *buf, size_t size, loff_t *flags)
{

    printk("--------^_^ %s------------\n",__FUNCTION__);

    return size;
}

long button_ioctl(struct file *filp, unsigned int cmd , unsigned long args)
{
    
    printk("--------^_^ %s------------\n",__FUNCTION__);
    
    return 0;
}

int button_close(struct inode *inode, struct file *filp)
{
    printk("--------^_^ %s------------\n",__FUNCTION__);
    
    return 0;
}


static struct file_operations fops = {
    .open = button_open,
    .read = button_read,
    .write = button_write,
    .unlocked_ioctl = button_ioctl,
    .release = button_close,
};


//加载函数和卸载函数
static int __init button_init(void)   //加载函数-----在驱动被加载时执行
{
    int ret;
    printk("--------^_^ %s------------\n",__FUNCTION__);
    //0,实例化设备对象
    //参数1 ---- 要申请的空间的大小
    //参数2 ---- 申请的空间的标识
    button_dev = kzalloc(sizeof(struct s5pv210_button),GFP_KERNEL);
    if(IS_ERR(button_dev)){
        printk("kzalloc error!\n");
        ret = PTR_ERR(button_dev);
        return -ENOMEM;
    }
    
    //1,申请设备号-----新方法
#if 0
    //静态申请设备号
    button_dev->major = 256;
    ret = register_chrdev_region(MKDEV(button_dev->major,0),1,"button_drv");
    if(ret < 0){
        printk("register_chrdev_region error!\n");
        ret =  -EINVAL;
        goto err_kfree;
    }
#else
    //动态申请设备号
    ret = alloc_chrdev_region(&button_dev->devno,0,1,"button_drv");
    if(ret < 0){
        printk("register_chrdev_region error!\n");
        ret =  -EINVAL;
        goto err_kfree;
    }
#endif

    //创建cdev

    //申请cdev的空间
    button_dev->cdev = cdev_alloc();
    if(IS_ERR(button_dev->cdev)){        
        printk("button_dev->cdev error!\n");
        ret = PTR_ERR(button_dev->cdev);
        goto err_unregister;
    }

    //初始化cdev的成员
    cdev_init(button_dev->cdev,&fops);

    //将cdev加入到内核中----链表
    ret = cdev_add(button_dev->cdev,button_dev->devno,1);


    
    /* 创建设备文件-----/dev/button */
    button_dev->cls = class_create(THIS_MODULE,"button_cls");
    if(IS_ERR(button_dev->cls)){
        printk("class_create error!\n");
        ret = PTR_ERR(button_dev->cls);
        goto err_cdev_del;
    }
    
    button_dev->dev = device_create(button_dev->cls,NULL,button_dev->devno,NULL,"button");
    if(IS_ERR(button_dev->dev)){
        printk("device_create error!\n");
        ret = PTR_ERR(button_dev->dev);
        goto err_class;
    }


    /* 硬件初始化---申请中断 */
    button_dev->irqno = IRQ_EINT(1);
    ret = request_irq(button_dev->irqno,button_irq_svc,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"eint-keydown",NULL);
    if(ret != 0){
        printk("request_irq error!\n");
        ret = -EBUSY;
        goto err_device;
    }

    return 0;
    
err_device:
    device_destroy(button_dev->cls,button_dev->devno);
err_class:
    class_destroy(button_dev->cls);
    
err_cdev_del:
    cdev_del(button_dev->cdev);
    
err_unregister:
    unregister_chrdev_region(button_dev->devno,1);
    
err_kfree:
    kfree(button_dev);
    return ret;

    
}

/* 卸载函数-----在驱动被卸载时执行 */
static void __exit button_exit(void)   
{
    printk("--------^_^ %s------------\n",__FUNCTION__);
    free_irq(button_dev->irqno,NULL);
    device_destroy(button_dev->cls,button_dev->devno);
    class_destroy(button_dev->cls);
    cdev_del(button_dev->cdev);
    unregister_chrdev_region(button_dev->devno,1);
    kfree(button_dev);
}

//声明和认证
module_init(button_init);
module_exit(button_exit);
MODULE_LICENSE("GPL");