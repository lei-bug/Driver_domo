#include <linux/init.h>             
#include <linux/module.h>          
#include <linux/kernel.h>   
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <asm/ioctl.h>
#include <linux/slab.h>

//#include <uapi/asm-generic/ioctl.h>
//#include <include/uapi/asm-generic/ioctl.h>

#define A1 _IO('L',0x11)
#define A2 _IOR('L',0x22,int)
#define A3 _IOW('L',0x33,int)
#define A4 _IOWR('L',0x44,int)

/*
#define A1 0x11
#define A2 0x22
#define A3 0x33
#define A4 0x44
*/

//指定license版本
MODULE_LICENSE("GPL");    

 char readbuf[1000];//读缓冲
 char writebuf[1000];//写缓冲
 char kerneldata[1000] = {"kernel data!"};

//打开设备
static int helloworld_open(struct inode *inode, struct file *filp){
    //printk(KERN_DEBUG "open");
    return 0;
}

//读取设备
static ssize_t helloworld_read(struct file *filp, char __user *buf,
size_t cnt, loff_t *offt){
    int ret = 0;
    printk(KERN_DEBUG "kerneldata=:%s",kerneldata);
    memcpy(readbuf,kerneldata,sizeof(kerneldata));
    ret = copy_to_user(buf,readbuf,cnt);
    if (ret==0){
    }
    return 0;
}

//写设备
static ssize_t helloworld_write(struct file *filp,
const char __user *buf,
size_t cnt, loff_t *offt){
int ret = 0;
//printk(KERN_DEBUG "write");
ret = copy_from_user(writebuf,buf,cnt);
if(ret==0){
    printk(KERN_DEBUG "kernel recevdata:%s\n",writebuf);

    strcat(kerneldata, writebuf);
    printk(KERN_DEBUG "kernel current_data:%s\n",kerneldata);
}
else{

}
    return 0;
}

//释放设备
static int helloworld_release(struct inode *inode, struct file *filp){
    //printk(KERN_DEBUG "close");
    return 0;
}


long helloworld_ioctl(struct file *filp, unsigned int cmd, unsigned long args){
    switch (cmd)
    {
        case A1 :
        printk(KERN_DEBUG "ioctl A1");
            break;
        case A2 :
        printk(KERN_DEBUG "ioctl A2");
            break;
        case A3 :
        printk(KERN_DEBUG "ioctl A3");
            break;
        case A4 :
        printk(KERN_DEBUG "ioctl A4");
            break;

        default :
            printk("unkown cmd\n");
            return -1;
            break;
    }
    return 0;
}

static const struct file_operations hello_fops = {
	.owner	= THIS_MODULE,
	.open	= helloworld_open,
	.read	= helloworld_read,
    .write  = helloworld_write,
    .release= helloworld_release,
    .unlocked_ioctl = helloworld_ioctl,
};

//设置初始化入口函数
static int __init hello_world_init(void)
{
    int ret = 0;
    /* 注册字符设备驱动 */
    ret = register_chrdev(201, "helloworld", &hello_fops);
    if(ret < 0){
        /* 字符设备注册失败,自行处理 */
       // printk(KERN_DEBUG "regester failed\n"); 
       printk(KERN_DEBUG "regester failed\n"); 
        return ret;
    }
    else{
        printk(KERN_DEBUG "regester succeed\n"); 
    }

/* 入口函数具体内容 */
    printk(KERN_DEBUG "----------hello world!!!---------\n");
    return 0;
}

//设置出口函数
static void __exit hello_world_exit(void)
{
    unregister_chrdev(201, "helloworld");
    printk(KERN_DEBUG "regester cancel");
    printk(KERN_DEBUG "---------goodbye world!!!---------\n");
}

//将上述定义的init()和exit()函数定义为模块入口/出口函数
module_init(hello_world_init);
module_exit(hello_world_exit);
