#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/kobject.h>   /* Necessary because we use sysfs */
#include <linux/device.h>
#include <mach/hardware.h>

#define sysfs_dir  "es6"
#define sysfs_file "pwm"
#define SYSFS_FILE_MACRO pwm

#define PWM1		0x4005C000
#define PWM2		0x4005C004

#define PWMEnable 	0x80000000
#define PWMFreqMax	0x0000FF00
#define PWMDuty		0x000000FF

#define sysfs_max_data_size 1024 /* due to limitations of sysfs, you mustn't go above PAGE_SIZE, 1k is already a *lot* of information for sysfs! */
static char sysfs_buffer[sysfs_max_data_size+1] = ""; /* an extra byte for the '\0' terminator */
static ssize_t used_buffer_size = 0;
static size_t regSz = 4;

static ssize_t
sysfs_store(struct device *dev,
			struct device_attribute *attr,
			const char *buffer,
			size_t count)
{	
	char command = 'x';
	int pwmNumber = -1;
	int dutyCycle = -1;
	int frequency = -1;
	sscanf(buffer, "%c %d %d %d", &command, &pwmNumber, &frequency);

	if (command == 'r') {
		//int pwm1Info = *io_p2v(PWM1);

		printk(KERN_INFO "hier, dingen, %x", *(unsigned int*)(io_p2v(PWM1)));
	}

	if (command == 'w') {
		printk(KERN_INFO "Kannie");
		
	}

	used_buffer_size = count > sysfs_max_data_size ? sysfs_max_data_size : count; /* handle MIN(used_buffer_size, count) bytes */
	
	memcpy(sysfs_buffer, buffer, used_buffer_size);
	sysfs_buffer[used_buffer_size] = '\0'; /* this is correct, the buffer is declared to be sysfs_max_data_size+1 bytes! */

	return used_buffer_size;
}


/* 
 * This line is now changed: in the previous example, the last parameter to DEVICE_ATTR
 * was NULL, now we add a store function as well. We must also add writing rights to the file:
 */
static DEVICE_ATTR(SYSFS_FILE_MACRO, S_IWUGO | S_IRUGO, NULL, sysfs_store);

/*
 * This is identical to previous example.
 */
static struct attribute *attrs[] = {
	&dev_attr_SYSFS_FILE_MACRO.attr,
	NULL   /* need to NULL terminate the list of attributes */
};
static struct attribute_group attr_group = {
	.attrs = attrs,
};
static struct kobject *hello_obj = NULL;


int __init sysfs_init(void)
{
	int result = 0;

	/*
	 * This is identical to previous example.
	 */
	hello_obj = kobject_create_and_add(sysfs_dir, kernel_kobj);
	if (hello_obj == NULL)
	{
		printk (KERN_INFO "%s module failed to load: kobject_create_and_add failed\n", sysfs_file);
		return -ENOMEM;
	}

	result = sysfs_create_group(hello_obj, &attr_group);
	if (result != 0)
	{
		/* creating files failed, thus we must remove the created directory! */
		printk (KERN_INFO "%s module failed to load: sysfs_create_group failed with result %d\n", sysfs_file, result);
		kobject_put(hello_obj);
		return -ENOMEM;
	}

	printk(KERN_INFO "/sys/kernel/%s/%s created\n", sysfs_dir, sysfs_file);
	return result;
}

void __exit sysfs_exit(void)
{
	kobject_put(hello_obj);
	printk (KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);
}

module_init(sysfs_init);
module_exit(sysfs_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("peekpoke");
