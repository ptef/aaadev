#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <linux/version.h>
#include <linux/fs.h>


/*
 * By changing this you choose how many devices to create
 */
#define MAX_DEV 2

#define MODULE_NAME	"aaadev"

//#define MY_MAJOR       42
//#define MY_MAX_MINORS  5

#define PRINTK_AFTER_MSGS    10240 // after N bytes printk in kernel log


static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations file_ops = {
    .owner      = THIS_MODULE,
    .open       = my_open,
    .release    = my_release,
    .unlocked_ioctl = my_ioctl,
    .read       = my_read,
    .write       = my_write
};

struct chardev_data_t {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *aaadev_class = NULL;
static struct chardev_data_t aaadev_data[MAX_DEV];

/*
 * This is the bytes we're initially going to print.
 * If the user writes a new 4-bytes array into the device, then we'll accept
 * and modify this array to the one received through write().
 */
static char data[4] = { 'A', 'A', 'A', 'A' };

/*
 * Although this is manually defined, it can be easily changed to get the size
 * automatically. But considering the purpose of the device I think it's safer
 * to statically define it.
 */
static size_t datalen = 4;

/*
 * types for kernel function pointers
 */
int (*sprint_symbol_ptr)(char *, unsigned long);
unsigned long (*kallsyms_lookup_name_ptr)(char *);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0)
static int my_uevent(const struct device *dev, struct kobj_uevent_env *env)
#else
static int my_uevent(struct device *dev, struct kobj_uevent_env *env)
#endif
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static unsigned long lookup_kernel_function_addr(char *f_name) {
    struct kprobe kp;
    unsigned long addr;

    memset(&kp, 0, sizeof(struct kprobe));
    kp.symbol_name = f_name;
    if (register_kprobe(&kp) < 0) {
        return 0;
    }
    addr = (unsigned long)kp.addr;
    unregister_kprobe(&kp);
    return addr;
}

static int __init my_init(void)
{
    int err, i;
    dev_t dev;
    unsigned long kaddr;

    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "aaadev");

    dev_major = MAJOR(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0)
    aaadev_class = class_create(MODULE_NAME);
#else
    aaadev_class = class_create(THIS_MODULE, MODULE_NAME);
#endif
    aaadev_class->dev_uevent = my_uevent;

    for (i=0; i < MAX_DEV; i++) {
        cdev_init(&aaadev_data[i].cdev, &file_ops);
        aaadev_data[i].cdev.owner = THIS_MODULE;

        cdev_add(&aaadev_data[i].cdev, MKDEV(dev_major, i), 1);

        device_create(aaadev_class, NULL, MKDEV(dev_major, i), NULL, "aaadev%d", i);
    }

    // technique 1
    kaddr = (unsigned long) &sprint_symbol;
    //kaddr &= 0xffffffffff000000;
    printk("AAADEV: JEAN: kaddr: %lx\n", kaddr);

    // technique 2 (prefered)
    kaddr = lookup_kernel_function_addr("sprint_symbol");
    printk("AAADEV: JEAN: kaddr: %lx\n", kaddr);

    kaddr = lookup_kernel_function_addr("kallsyms_lookup_name");
    kallsyms_lookup_name_ptr = (unsigned long (*)(char *))kaddr;
    if (kallsyms_lookup_name_ptr)
	    printk("AAADEV: JEAN: kaddr: %p\n", kallsyms_lookup_name_ptr);
    else
	    printk(KERN_INFO "JEAN: no!\n");

    return 0;
}

static void __exit my_exit(void)
{
    int i;

    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(aaadev_class, MKDEV(dev_major, i));
    }

    class_unregister(aaadev_class);
    class_destroy(aaadev_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static int my_open(struct inode *inode, struct file *file)
{
    printk("AAADEV: Device open\n");

    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk("AAADEV: Device close\n");
    return 0;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk("AAADEV: Device ioctl\n");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    static int printk_after_reads = 0;

    if (printk_after_reads < PRINTK_AFTER_MSGS) {
        printk_after_reads++;
    } else {
        printk("Reading device: %d. Read %d bytes\n", MINOR(file->f_path.dentry->d_inode->i_rdev), printk_after_reads);
        printk_after_reads = 0;
    }

    if (count > datalen) {
        count = datalen;
    }

    if (copy_to_user(buf, data, count)) {
        return -EFAULT;
    }

    return count;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    static int printk_after_writes = 0;
    size_t maxdatalen = datalen;
    size_t copy_success = 0;

    if (count < maxdatalen) {
        maxdatalen = count;
    }

   copy_success = copy_from_user(data, buf, maxdatalen);

    if (copy_success == 0) {
        if (printk_after_writes < 2) { // I think it's stupid to force user to write() twice, anyways...
            printk_after_writes++;
        } else {
            printk("Writing device: %d. Wrote %ld bytes\n", MINOR(file->f_path.dentry->d_inode->i_rdev), maxdatalen);
            printk_after_writes = 0;
        }
    } else {
        printk("Could't copy %zd bytes from the user\n", copy_success);
    }

    return count;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dukpt <j@bsd.com.br>");
MODULE_DESCRIPTION("Print 0x41414141 characters");
MODULE_VERSION("1.0");

module_init(my_init);
module_exit(my_exit);
