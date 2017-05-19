#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>

/*
 * Joystick defines / GPIO
 */
#define Press     1  // J3.47 (9th pin from bottom/left)
#define Down      16 // J3.49 (8th pin from bottom/left)
#define Right     8  // J3.57 (4th pin from bottom/left)
#define Left      2  // J3.56 (5th pin from bottom/right)
#define Up        4  // J3.48 (9th pin from bottom/right)

/*
 * These register values to control GPIO are from the LPC32x0 User Manual
 */
// in the init thing
#define P2_MUX_SET          0x40028028  // WO   Write '1' to bit 3 to set EMC_D[31:19] pins being configured as GPIO pins P2[12:0]
#define P2_MUX_CLR          0x4002802C  // WO   Write '1' to clear P2 MUX STATE
#define P2_MUX_STATE        0x40028030  // RO   Is 0 bit

// this will come in /dev/
#define P2_INP_STATE        0x4002801C  // RO   Reads P2.[bit] state
#define P2_OUTP_SET         0x40028020  // WO   Writes same
#define P2_OUTP_CLR         0x40028024  // WO   Write '1' to drive P2.[bit] low

// 0 is input, 1 is output
// this is /sys/
#define P2_DIR_SET          0x40028010  // WO   Write '1' to set P2.[bit] to Output
#define P2_DIR_CLR          0x40028014  // WO   Write '1' to set P2.[bit] to Input
#define P2_DIR_STATE        0x40028018  // RO   Reads P2.[bit] direction

/*
 * sysfs definitions
 */
#define sysfs_dir           "es6"
#define sysfs_file          "gpio"
#define SYSFS_FILE_MACRO    gpiofs

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * devfs definitions
 */
#define DEVICE_NAME         "gpio"
#define DEVICE_MAJOR        137

#define BUF_LEN             80

#define sysfs_max_data_size 1024 /* due to limitations of sysfs, you mustn't go above PAGE_SIZE, 1k is already a *lot* of information for sysfs! */
static char sysfs_buffer[sysfs_max_data_size+1] = ""; /* an extra byte for the '\0' terminator */
static ssize_t used_buffer_size = 0;
static size_t regSz = 4; // because documentation

char result_buffer[sysfs_max_data_size+1] = "";


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
    
    //sscanf(Message_Ptr, "%d", &valueToWrite);

    /*if(valueToWrite < 0 && valueToWrite > 255) {
        printk(KERN_INFO "nope...");
        return len;
    }*/

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
    unsigned int address = 0;
    unsigned int value = 0;
    sscanf(buffer, "%c %x %d", &command, &address, &value);

    // Read value registers starting from address
    if (command == 'r') {
        int i = 0;
        printk(KERN_INFO "r: Address: %x, Length: %d\n", address, value);
        for (i = 0; i < value; i++) {
            printk(KERN_INFO "r: Offset: %d Result: %u\n", i, *(unsigned int*)(io_p2v(address + i * regSz)));
        }
    }

    // echo "r 40024000 2" > /sys/kernel/es6/data
    // Gives the up and down counters

    // Write whatever is value to address (still just an int)
    if (command == 'w') {
        printk(KERN_INFO "w: Address: %x, Length: %d\n", address, value);
        memcpy(io_p2v(address),&value,sizeof(unsigned int));
    }

    // We can write to 0x400A8014 and read it back.

    used_buffer_size = count > sysfs_max_data_size ? sysfs_max_data_size : count; /* handle MIN(used_buffer_size, count) bytes */
    
    //printk(KERN_INFO "sysfile_write (/sys/kernel/%s/%s) called, buffer: %s, count: %d\n", sysfs_dir, sysfs_file, buffer, count);

    memcpy(sysfs_buffer, buffer, used_buffer_size);
    sysfs_buffer[used_buffer_size] = '\0'; /* this is correct, the buffer is declared to be sysfs_max_data_size+1 bytes! */

    return used_buffer_size;
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

int sysfs_init(void) {
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

void sysfs_exit(void) {
    kobject_put(gpio_kobj);
    printk (KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);
}


int devfs_init(void) {
    int major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);

    if (major < 0) {
        printk ("Registering the character device failed");
        return major;
    }
    printk("Registering the character device succesfull");
    // init devfs

    return 0;
}


void devfs_exit(void) {
    unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}

void __init gpio_init() {
    sysfs_init();
    devfs_init();
}

void __exit gpio_exit() {
    sysfs_exit();
    devfs_exit();
}





module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("gpio");
