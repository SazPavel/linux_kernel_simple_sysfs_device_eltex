#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/rwlock.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sazonov Pavel");
MODULE_DESCRIPTION("A simple sysfs device");
MODULE_VERSION("0.2");

static DEFINE_RWLOCK(simple_device_lock);

static ssize_t simple_show(struct kobject *kobj, struct kobj_attribute *attr,
                           char *buf);
static ssize_t simple_store(struct kobject *kobj, struct kobj_attribute *attr,
                            const char *buf, size_t n);
static unsigned int simple_hook(void *priv, struct sk_buff *skb,
                                const struct nf_hook_state *state);


static int port = 1111;
static struct kobject *simple_kobj;
static struct kobj_attribute simple_attr = {
    .attr = {.name = "simple_device", .mode = 0666},
    .store = simple_store,
    .show = simple_show,
};

static const struct nf_hook_ops simple_ops = {
    .hook = simple_hook,
    .pf = NFPROTO_INET,
    .hooknum = NF_INET_LOCAL_OUT,
    .priority = NF_IP_PRI_FIRST,
};

static ssize_t simple_show(struct kobject *kobj, struct kobj_attribute *attr,
                           char *buf)
{
    int ret;
    read_lock(&simple_device_lock);
    ret = snprintf(buf, sizeof(port) + 2, "%d\n", port);
    read_unlock(&simple_device_lock);
    return ret;
}

static ssize_t simple_store(struct kobject *kobj, struct kobj_attribute *attr,
                            const char *buf, size_t len)
{
    ssize_t ret;
    write_lock(&simple_device_lock);
    ret = sscanf(buf, "%d", &port);
    write_unlock(&simple_device_lock);
    if(ret > 0)
    {
        printk(KERN_INFO "new blocked port: %d\n", port);
    }
    return len;
}

static unsigned int simple_hook(void *priv, struct sk_buff *skb,
                                const struct nf_hook_state *state)
{
    struct udphdr *UDPHead;
    //If not IP
    if(skb->protocol != htons(ETH_P_IP))
        return NF_ACCEPT;
    //If not UDP
    if(ip_hdr(skb)->protocol != IPPROTO_UDP)
        return NF_ACCEPT;
    if(!(UDPHead = udp_hdr(skb)))
        return NF_ACCEPT;
    if(UDPHead->dest==htons(port))
    {
        printk(KERN_INFO "block\n");
        return NF_DROP;
    }
    return NF_ACCEPT;
}

static int __init simple_device_init(void)
{
    int ret;
    printk(KERN_INFO "Loading simple device module\n");
    simple_kobj = kobject_create_and_add("simple device", NULL);
    if(!simple_kobj)
        return -ENOMEM;
    ret = sysfs_create_file(simple_kobj, &simple_attr.attr);
    if(ret)
    {
        kobject_put(simple_kobj);
	return ret;
    }
    ret = nf_register_net_hook(&init_net, &simple_ops);
    if(ret < 0)
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
    nf_unregister_net_hook(&init_net, &simple_ops);
    printk(KERN_INFO "Simple device module unload\n");
}

module_init(simple_device_init);
module_exit(simple_device_exit);
