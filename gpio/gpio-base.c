#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>

#include "regs.h"
#include "ports.h"

/*
 * sysfs definitions
 */
#define sysfs_dir           "es6_gpio"
#define sysfs_file          "gpio"
#define SYSFS_FILE_MACRO    gpiofs

/*
 * devfs definitions
 */
#define DEVICE_NAME         "es6_gpio"
#define DEVICE_MAJOR        137

#define BUF_LEN             80

#define sysfs_max_data_size 1024 /* due to limitations of sysfs, you mustn't go above PAGE_SIZE, 1k is already a *lot* of information for sysfs! */
static char sysfs_buffer[sysfs_max_data_size+1] = ""; /* an extra byte for the '\0' terminator */

char result_buffer[sysfs_max_data_size+1] = "";

static int deviceIsOpen = 0;
static char Message[BUF_LEN];
static char *Message_Ptr;
static PortInfo selectedPort;

///////////////////////////////////////////////////////////////////////////////////////////////////
// devfs section
///////////////////////////////////////////////////////////////////////////////////////////////////

static int device_open(struct inode *inode, struct file *file) {
    int minor = MINOR(inode->i_rdev);

    Message_Ptr = Message;
    
    file->private_data = (void*)minor;

    if (deviceIsOpen) return -EBUSY;
    deviceIsOpen++;

    // printk(KERN_INFO "Major: %d, Minor: %d", MAJOR(inode->i_rdev), minor);

    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    deviceIsOpen--;
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset) {
    int bytes_read = 0;
    int minor = (int)filp->private_data;
    int jumper, port, direction, inputstate;
    unsigned int rawDirection;
    unsigned int rawInputState;

    if (minor != 0) {
        printk(KERN_ERR "Wrong minor number, expected 0");
        return -EINVAL;
    }

    if (selectedPort.Bit < 1) {
        printk(KERN_INFO "No port selected");
    }

    if (*Message_Ptr == '\0') {
        return 0;
    }

    rawDirection = *(unsigned int*)(io_p2v(selectedPort.RegDIR + STATE_OFFSET));
    rawInputState = *(unsigned int*)(io_p2v(selectedPort.RegINP));
    jumper = selectedPort.Jumper;
    port = selectedPort.PhysicalPin;
    
    direction = rawDirection & selectedPort.Bit ? 0 : 1;// >> selectedPort.Bit;
    inputstate = rawInputState & selectedPort.Bit ? 0 : 1;// >> selectedPort.Bit;

    sprintf(Message_Ptr, "J%d.%d Direction: %u Input: %u", jumper, port, direction, inputstate);

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
    char command = 'x';
    unsigned int jumper = 0;
    unsigned int pin = 0;
    PortInfo port;
    int Message_Index;

    if (minor != 0) {
        printk(KERN_ERR "Wrong minor number, expected 0");
        return -EINVAL;
    }

    for (Message_Index = 0; Message_Index < len && Message_Index < BUF_LEN; Message_Index++) {
        get_user(Message[Message_Index], buff + Message_Index);
    }

    Message_Ptr = Message;
    

    if (sscanf(Message_Ptr, "%c J%d.%d", &command, &jumper, &pin) != 3) {
        printk(KERN_ERR "Wrong input. Expected: [r, h, l] J[jumper].[pin]");
        return -EINVAL;        
    }

    if (jumper < 1 || jumper > 3) {
        printk(KERN_ERR "Unsupported jumper");
        return -EINVAL;
    }

    port = GetJumperPinVal(jumper, pin, false);

    if (port.Bit < 1) { 
        printk(KERN_ERR "J%d.%d is not supported for GPIO", jumper, pin);
        return -EINVAL;
    }

    if (command == 'r') {
        printk(KERN_INFO "J%d.%d set for read", jumper, pin);
        // These pins use different bits, hence this check
        if ((jumper == 1 && pin == 24) ||
            (jumper == 3 && pin == 54) ||
            (jumper == 3 && pin == 46) ||
            (jumper == 3 && pin == 36)) {
            selectedPort = GetJumperPinVal(jumper, pin, true);
        }
        else {
            selectedPort = port;
        }
    }
    else if (command == 'h') {
        memcpy(io_p2v(port.RegOUT + SET_OFFSET),&port.Bit,sizeof(unsigned int));
        printk(KERN_INFO "J%d.%d high", jumper, pin);
    }
    else if (command == 'l') {
        memcpy(io_p2v(port.RegOUT + CLR_OFFSET),&port.Bit,sizeof(unsigned int));
        printk(KERN_INFO "J%d.%d low", jumper, pin);
    }
    else {
        printk(KERN_ERR "Invalid command");
        return -EINVAL;
    }

    return Message_Index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// sysfs section
///////////////////////////////////////////////////////////////////////////////////////////////////
static ssize_t
sysfs_show(struct device *dev,
           struct device_attribute *attr,
           char *buffer)
{
    printk(KERN_INFO "sysfile_read (/sys/kernel/%s/%s) called\n", sysfs_dir, sysfs_file);
    
    /*
     * The only change here is that we now return sysfs_buffer, rather than a fixed HelloWorld string.
     */
    return sprintf(buffer, "%s", sysfs_buffer);
}

static ssize_t
sysfs_store(struct device *dev,
            struct device_attribute *attr,
            const char *buffer,
            size_t count)
{   
    char command = 'x';
    unsigned int jumper = 0;
    unsigned int pin = 0;
    PortInfo port;

    if (sscanf(buffer, "%c J%d.%d", &command, &jumper, &pin) != 3) {
        printk(KERN_ERR "Wrong input. Expected: [i, o] J[jumper].[pin]");
        return -EINVAL;        
    }

    if (jumper < 1 || jumper > 3) {
        printk(KERN_ERR "Unsupported jumper");
        return -EINVAL;
    }

    // A valid bit should be >= 1
    port = GetJumperPinVal(jumper, pin, false);

    if (port.Bit < 1) { 
        printk(KERN_ERR "J%d.%d is not supported for GPIO", jumper, pin);
        return -EINVAL;
    }

    // TODO : Use base + offset instead of hardcoded to allow for
    // an arbitrary register 
    if (command == 'i') {
        printk(KERN_INFO "J%d.%d set to INPUT", jumper, pin);
        memcpy(io_p2v(port.RegDIR + CLR_OFFSET),&port.Bit,sizeof(unsigned int));
    }
    else if (command == 'o') {
        printk(KERN_INFO "J%d.%d set to OUTPUT", jumper, pin);
        memcpy(io_p2v(port.RegDIR + SET_OFFSET),&port.Bit,sizeof(unsigned int));
    }
    else {
        printk(KERN_ERR "Invalid command");
        return -EINVAL;
    }

    return count;
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

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

void gpio_init_ports(void) {
    *(unsigned int*)(io_p2v(P2_MUX_SET)) = VAL_MUX;
    *(unsigned int*)(io_p2v(REG_LCDCONFIG)) = VAL_LCDCONFIG;
}

int __init gpio_init(void) {
    int result = 0;

    int major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);
    
    if (major < 0) {
        printk (KERN_ERR "Registering the character device failed\n");
        return major;
    }

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
    printk(KERN_INFO "Registering chardev success, run:\n");
    printk(KERN_INFO "mknod /dev/%s c %d [minor_number]\n",DEVICE_NAME, DEVICE_MAJOR);

    gpio_init_ports();
    return result;
}

void __exit gpio_exit(void) {
    kobject_put(gpio_kobj);
    printk (KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);

    unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);    
}

module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("gpio");
