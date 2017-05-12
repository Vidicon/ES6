#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>

/*
 * These register values are from the LPC32x0 User Manual
 */

#define P2_MUX_SET      0x40028028  // WO   Write '1' to bit 3 to set EMC_D[31:19] pins being configured as GPIO pins P2[12:0]
#define P2_MUX_CLR      0x4002802C  // WO   Write '1' to clear P2 MUX STATE
#define P2_MUX_STATE    0x40028030  // RO   Is 0 bit

#define P2_INP_STATE    0x4002801C  // RO   Reads P2.[bit] state
#define P2_OUTP_SET     0x40028020  // WO   Writes same
#define P2_OUTP_CLR     0x40028024  // WO   Write '1' to drive P2.[bit] low
#define P2_DIR_SET      0x40028010  // WO   Write '1' to set P2.[bit] to Output
#define P2_DIR_CLR      0x40028014  // WO   Write '1' to set P2.[bit] to Input
#define P2_DIR_STATE    0x40028018  // RO   Reads P2.[bit] direction


/*
 * Defines
 */
#define sysfs_dir       "es6"
#define sysfs_file      "gpio"
#define SYSFS_FILE_MACRO gpiofs




static DEVICE_ATTR(SYSFS_FILE_MACRO, S_IWUGO | S_IRUGO, sysfs_show, sysfs_store);
static struct attribute *attrs[] = {
    &dev_attr_SYSFS_FILE_MACRO.attr,
    NULL   /* need to NULL terminate the list of attributes */
};
static struct attribute_group attr_group = {
    .attrs = attrs,
};
static struct kobject *gpio_kobj = NULL;

int __init sysfs_init(void)
{
    int result = 0;

    gpio_kobj = kobject_create_and_add(sysfs_dir, kernel_kobj);
    if (gpio_kobj == NULL)
    {
        printk (KERN_INFO "%s module failed to load: kobject_create_and_add failed\n", sysfs_file);
        return -ENOMEM;
    }

    result = sysfs_create_group(gpio_kobj, &attr_group);
    if (result != 0)
    {
        printk (KERN_INFO "%s module failed to load: sysfs_create_group failed with result %d\n", sysfs_file, result);
        kobject_put(gpio_kobj);
        return -ENOMEM;
    }

    printk(KERN_INFO "/sys/kernel/%s/%s created\n", sysfs_dir, sysfs_file);
    return result;
}

void __exit sysfs_exit(void)
{
    kobject_put(gpio_kobj);
    printk (KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);
}

module_init(sysfs_init);
module_exit(sysfs_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("gpio");
