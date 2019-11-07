#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/rwlock.h>

#define EXAMPLE_MSG "HELLO WORLD!\n"
#define MSG_BUF_LEN 15

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sazonov Pavel");
MODULE_DESCRIPTION("A simple sysfs device");
MODULE_VERSION("0.1");

static DEFINE_RWLOCK(simple_device_lock);

static ssize_t simple_show(struct kobject *kobj, struct kobj_attribute *attr,
                           char *buf);
static ssize_t simple_store(struct kobject *kobj, struct kobj_attribute *attr,
                            const char *buf, size_t n);

static int	simple_size = MSG_BUF_LEN;
static char	simple_data[MSG_BUF_LEN];
static struct kobject *simple_kobj;
static struct kobj_attribute simple_attr = {
    .attr = {.name = "simple_device", .mode = 0666},
    .store = simple_store,
    .show = simple_show,
};

static ssize_t simple_show(struct kobject *kobj, struct kobj_attribute *attr,
                           char *buf)
{
    int ret;
    read_lock(&simple_device_lock);
    ret = snprintf(buf, MSG_BUF_LEN, "%s\n", simple_data);
    read_unlock(&simple_device_lock);
    return ret;
}

static ssize_t simple_store(struct kobject *kobj, struct kobj_attribute *attr,
                            const char *buf, size_t len)
{
    ssize_t ret;
    char *temp_buf;
    temp_buf = kmalloc(len, GFP_KERNEL);
    if(!temp_buf)
        return -ENOMEM;
    write_lock(&simple_device_lock);
    ret = sscanf(buf, "%s", temp_buf);
    write_unlock(&simple_device_lock);
    if(ret > 0)
    {
        printk(KERN_INFO "buf: %s\n", temp_buf);
    }
    kfree(temp_buf);
    return len;
}

static int __init simple_device_init(void)
{
    int ret;
    printk(KERN_INFO "Loading simple device module\n");
    strncpy(simple_data, EXAMPLE_MSG, simple_size);
    simple_kobj = kobject_create_and_add("simple_device", NULL);
    if(!simple_kobj)
        return -ENOMEM;
    ret = sysfs_create_file(simple_kobj, &simple_attr.attr);
    if(ret)
    {
        kobject_put(simple_kobj);
	return ret;
    }
    printk(KERN_INFO "Load simple device module successful\n");
    return 0;
}

static void __exit simple_device_exit(void)
{
    kobject_put(simple_kobj);
    printk(KERN_INFO "Simple device module unload\n");
}

module_init(simple_device_init);
module_exit(simple_device_exit);
