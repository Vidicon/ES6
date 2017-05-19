#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>

/*
 * These register values to control GPIO are from the LPC32x0 User Manual
 */
#define P2_MUX_SET      0x40028028  // WO   Write '1' to bit 3 to set EMC_D[31:19] pins being configured as GPIO pins P2[12:0]
#define P2_MUX_CLR      0x4002802C  // WO   Write '1' to clear P2 MUX STATE
#define P2_MUX_STATE    0x40028030  // RO   Is 0 bit

#define P2_INP_STATE    0x4002801C  // RO   Reads P2.[bit] state
#define P2_OUTP_SET     0x40028020  // WO   Writes same
#define P2_OUTP_CLR     0x40028024  // WO   Write '1' to drive P2.[bit] low

// 0 is input, 1 is output
#define P2_DIR_SET      0x40028010  // WO   Write '1' to set P2.[bit] to Output
#define P2_DIR_CLR      0x40028014  // WO   Write '1' to set P2.[bit] to Input
#define P2_DIR_STATE    0x40028018  // RO   Reads P2.[bit] direction

/*
 * sysfs definitions
 */
#define sysfs_dir       "es6"
#define sysfs_file      "gpio"
#define SYSFS_FILE_MACRO gpiofs

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * devfs definitions
 */
#define DEVICE_NAME         "gpio"
#define DEVICE_MAJOR        137

#define BUF_LEN             80

/*
 * iets met enums
 */
enum devType {
    lezen = 0,
    schrijven = 1,
};

static int deviceIsOpen = 0;
static char Message[BUF_LEN];
static char *Message_Ptr;

static int device_open(struct inode *inode, struct file *file) {
    int minor = MINOR(inode->i_rdev);

    Message_Ptr = Message;
    
    file->private_data = (void*)minor;

    if (deviceIsOpen) return -EBUSY;
    deviceIsOpen++;

    printk(KERN_INFO "Major: %d, minor: %d", MAJOR(inode->i_rdev), minor);

    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    deviceIsOpen--;
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset) {
    int bytes_read = 0;
    int minor = (int)filp->private_data;

    int PWM1Value = *(unsigned int*)(io_p2v(REG_PWM1));
    int PWM2Value = *(unsigned int*)(io_p2v(REG_PWM2));

    // hier een schakel kees ofzo

    // cpy_to_usr somehow
    while (length && *Message_Ptr) {
        put_user(*(Message_Ptr++), buffer++);
        length--;
        bytes_read++;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
    int minor = (int)filp->private_data;
    
    int i;
    // copy_from_user
    for (i = 0; i < len && i < BUF_LEN; i++) {
        get_user(Message[i], buff + i);
    }

    Message_Ptr = Message;
    
    sscanf(Message_Ptr, "%d", &valueToWrite);

    if(valueToWrite < 0 && valueToWrite > 255) {
        printk(KERN_INFO "nope...");
        return len;
    }

    //schakelcasus
    return i;
}

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

///////////////////////////////////////////////////////////////////////////////////////////////////

int (*SetFuncs[])(unsigned int reg, int value) {
    set_direction,
    set_output,
    read_direction,
    read_input,
}


static DEVICE_ATTR(SYSFS_FILE_MACRO, S_IWUGO | S_IRUGO, sysfs_show, sysfs_store);
static struct attribute *attrs[] = {
    &dev_attr_SYSFS_FILE_MACRO.attr,
    NULL   /* need to NULL terminate the list of attributes */
};
static struct attribute_group attr_group = {
    .attrs = attrs,
};
static struct kobject *gpio_kobj = NULL;

int __init sysfs_init(void) {
    int result = 0;

    gpio_kobj = kobject_create_and_add(sysfs_dir, kernel_kobj);
    if (gpio_kobj == NULL) {
        printk (KERN_INFO "%s module failed to load: kobject_create_and_add failed\n", sysfs_file);
        return -ENOMEM;
    }

    result = sysfs_create_group(gpio_kobj, &attr_group);
    if (result != 0) {
        printk (KERN_INFO "%s module failed to load: sysfs_create_group failed with result %d\n", sysfs_file, result);
        kobject_put(gpio_kobj);
        return -ENOMEM;
    }

    printk(KERN_INFO "/sys/kernel/%s/%s created\n", sysfs_dir, sysfs_file);
    return result;
}

void __exit sysfs_exit(void) {
    kobject_put(gpio_kobj);
    printk (KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);
}





int init_module(void) {
    int major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);

    if (major < 0) {
        printk ("Registering the character device failed");
        return major;
    }
    printk("Registering the character device succesfull");
    // init devfs

    return 0;
}


void cleanup_module(void) {
    unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}





module_init(sysfs_init);
module_exit(sysfs_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("gpio");
