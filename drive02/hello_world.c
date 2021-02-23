#include<linux/init.h>
#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>
#include<linux/string.h>

/*属性文件*/
static int hello_value;
int retval;

/*read*/
static ssize_t hello_show(struct kobject *kobj, struct kobj_attribute *attr,
                          char *buf){
   return sprintf(buf, "%d\n",hello_value);
}

/* write */
static ssize_t hello_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count){
   sscanf(buf, "%d", &hello_value);
   return count;
}

/*定义Kobject*/
static struct kobject *helloworld_kobj;

/*定义Kobj属性*/
static struct kobj_attribute hello_value_attribute = __ATTR(hello_value,0666,hello_show,hello_store);

/*入口函数*/
static int __init helloworld_init(void){
       printk(KERN_DEBUG "---------hello world!!!-------sysfs----\n");
   helloworld_kobj = kobject_create_and_add("helloworld",kernel_kobj);
   //创建helloworld的目录项
   if(!helloworld_kobj)
      return -ENOMEM;//返回错误码
      retval = sysfs_create_file(helloworld_kobj,&hello_value_attribute);
      //建立文件与操作之间的联系和对应
   if(retval)
      kobject_put(helloworld_kobj);
   return retval;
}
 
/*出口函数*/
static void __exit helloworld_exit(void){
   kobject_put(helloworld_kobj);
   printk(KERN_DEBUG "-------goodbye world!!!--------sysfs-----\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);
MODULE_LICENSE("GPL");