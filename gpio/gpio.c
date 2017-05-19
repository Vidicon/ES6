#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>

#include "ports.h"

/*
 * P2 GPIO control registers
 */
// in the init thing
#define P2_MUX_SET          0x40028028  // WO   Write '1' to bit 3 to set EMC_D[31:19] pins being configured as GPIO pins P2[12:0]
#define P2_MUX_CLR          0x4002802C  // WO   Write '1' to clear P2 MUX STATE
#define P2_MUX_STATE        0x40028030  // RO   Is 0 bit

// 0 is input, 1 is output
// this is /sys/
#define P2_DIR_SET          0x40028010  // WO   Write '1' to set P2.[bit] to Output
#define P2_DIR_CLR          0x40028014  // WO   Write '1' to set P2.[bit] to Input
#define P2_DIR_STATE        0x40028018  // RO   Reads P2.[bit] direction

// this will come in /dev/
#define P2_INP_STATE        0x4002801C  // RO   Reads P2.[bit] input state
#define P2_OUTP_SET         0x40028020  // WO   Writes P2.[bit] output state
#define P2_OUTP_CLR         0x40028024  // WO   Write '1' to drive P2.[bit] low

/*
 * sysfs definitions
 */
#define sysfs_dir           "es6_gpio"
#define sysfs_file          "gpio"
#define SYSFS_FILE_MACRO    gpiofs

///////////////////////////////////////////////////////////////////////////////////////////////////
// devfs section
///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * devfs definitions
 */
#define DEVICE_NAME         "gpio"
#define DEVICE_MAJOR        137

#define BUF_LEN             80

#define sysfs_max_data_size 1024 /* due to limitations of sysfs, you mustn't go above PAGE_SIZE, 1k is already a *lot* of information for sysfs! */
static char sysfs_buffer[sysfs_max_data_size+1] = ""; /* an extra byte for the '\0' terminator */

char result_buffer[sysfs_max_data_size+1] = "";

static int deviceIsOpen = 0;
static char Message[BUF_LEN];
static char *Message_Ptr;

int currentPin;

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
    
    switch(minor) {
        /*case 0: {
            bool enabled = (PWM1Value & MASK_PWM_ENABLE) != 0;          
            sprintf(Message_Ptr, "PWM1 enabled: %s", enabled ? "Yes" : "No");
            break;
        }
        case 1: {
            int reloadValue = (PWM1Value & MASK_PWM_FREQUENCY) >> PWM_SHIFT_FREQ;
            if(reloadValue == 0) {
                sprintf(Message_Ptr, "nope..."); // iets met delen door 0
                break;
            }
            sprintf(Message_Ptr, "PWM1 raw: %d, freq: %dHz", reloadValue, map_freq(reloadValue));
            break;
        }
        case 2: {
            int dutyValue = (PWM1Value & MASK_PWM_DUTY);
            if(dutyValue == 0) {
                sprintf(Message_Ptr, "nope..."); // iets met delen door 0
                break;
            }
            sprintf(Message_Ptr, "PWM1 raw: %d, duty: %d%%", dutyValue, map_duty(dutyValue));
            break;
        }
        case 3: {
            bool enabled = (PWM2Value & MASK_PWM_ENABLE) != 0;
            sprintf(Message_Ptr, "PWM2 enabled: %s", enabled ? "Yes" : "No");
            break;
        }
        case 4: {
            int reloadValue = (PWM2Value & MASK_PWM_FREQUENCY) >> PWM_SHIFT_FREQ;
            if(reloadValue == 0) {
                sprintf(Message_Ptr, "nope..."); // iets met delen door 0
                break;
            }
            sprintf(Message_Ptr, "PWM2 raw: %d, freq: %dHz", reloadValue, map_freq(reloadValue));
            break;
        }
        case 5: {
            int dutyValue = (PWM2Value & MASK_PWM_DUTY);
            if(dutyValue == 0) {
                sprintf(Message_Ptr, "nope..."); // iets met delen door 0
                break;
            }
            sprintf(Message_Ptr, "PWM2 raw: %d, duty: %d%%", dutyValue, map_duty(dutyValue));
            break;
        }*/
        default:
            sprintf(Message_Ptr, "nope break...");
            break;
    }

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
    
    //sscanf(Message_Ptr, "%d", &valueToWrite);

    /*if(valueToWrite < 0 && valueToWrite > 255) {
        printk(KERN_INFO "nope...");
        return len;
    }*/

    //schakelcasus
    return i;
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
    int bitToSet = -1;
    int allBits = 0;

    if (sscanf(buffer, "%c %d %d", &command, &jumper, &pin) != 3) {
        printk(KERN_ERR "Wrong input. Expected format: [i, o] [jumper number] [pin number]");
        return -EINVAL;        
    }

    if (jumper < 1 || jumper > 3) {
        printk(KERN_ERR "Unsupported jumper");
        return -EINVAL;
    }

    // A valid bit should be >= 1
    // Error = -1
    // Consider what to do with 0 (our protocol to set all????)
    bitToSet = GetJumperPinVal(jumper, pin);
    allBits = 0 | bitToSet; // find shit

    if (bitToSet < 0) { 
        printk(KERN_ERR "J%d.%d is not supported for GPIO", jumper, pin);
        return -EINVAL;
    }

    if (command == 'i') {
        printk(KERN_INFO "J%d.%d set to INPUT", jumper, pin);
        memcpy(io_p2v(P2_DIR_CLR),&allBits,sizeof(unsigned int));
    }
    else if (command == 'o') {
        printk(KERN_INFO "J%d.%d set to OUTPUT", jumper, pin);
        memcpy(io_p2v(P2_DIR_SET),&allBits,sizeof(unsigned int));
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

int __init gpio_init(void) {
    int result = 0;
    int major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);

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

    if (major < 0) {
        printk ("Registering the character device failed");
        return major;
    }
    printk("Registering the character device succesfull");

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
