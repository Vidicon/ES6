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
#define PWMCLOCK	0x400040B8	//= 0x115
#define LCDCONFIG	0x40004054	//= 0
#define PWMEnable 	0x80000000
#define PWMBitsToShiftForFreq 8 // lekker korte naam toch ;p

#define DEV_NAME "pwm_device"
#define DEV_MAJOR	99

static ssize_t
sysfs_pwm(struct device *dev,
			struct device_attribute *attr,
			const char *buffer,
			size_t count)
{	
	char command = 'x';
	int pwmNumber = -1;
	int dutyCycle = -1;
	int frequency = -1;
	sscanf(buffer, "%c %d %d %d", &command, &pwmNumber, &frequency, &dutyCycle);

	if (command == 'r') {
		if(pwmNumber == 1) {
			printk(KERN_INFO "pwm1: %x\n", *(unsigned int*)(io_p2v(PWM1)));
		}
		else if(pwmNumber == 2) {
			printk(KERN_INFO "pwm2: %x\n", *(unsigned int*)(io_p2v(PWM2)));
		}
		else {
			printk(KERN_INFO "doe eens niet...\n");
		}
	} else if (command == 'w') {
		if(pwmNumber == 1) {
			if(frequency >= 0 && frequency < 256 && dutyCycle >= 0 && dutyCycle < 256) {
				*(unsigned int*)(io_p2v(PWM1)) = PWMEnable;
				*(unsigned int*)(io_p2v(PWM1)) |= frequency << PWMBitsToShiftForFreq;
				*(unsigned int*)(io_p2v(PWM1)) |= dutyCycle;
				printk(KERN_INFO "pwm1 geschreven\n");
			} else {
				printk(KERN_INFO "nope...\n");
			}
			
		}
		else if(pwmNumber == 2) {
			if(frequency >= 0 && frequency < 256 && dutyCycle >= 0 && dutyCycle < 256) {
				*(unsigned int*)(io_p2v(PWM2)) = PWMEnable;
				*(unsigned int*)(io_p2v(PWM2)) |= frequency << PWMBitsToShiftForFreq;
				*(unsigned int*)(io_p2v(PWM2)) |= dutyCycle;
				printk(KERN_INFO "pwm2 geschreven\n");
			} else {
				printk(KERN_INFO "nope...\n");
			}
		} else {			
			printk(KERN_INFO "doe eens niet...\n");
		}
	} else {
		printk(KERN_INFO "doe eens niet...\n");
	}

	return 0;
}


/* 
 * This line is now changed: in the previous example, the last parameter to DEVICE_ATTR
 * was NULL, now we add a store function as well. We must also add writing rights to the file:
 */
static DEVICE_ATTR(SYSFS_FILE_MACRO, S_IWUGO | S_IRUGO, NULL, sysfs_pwm);

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
static struct kobject *pwm_obj = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/fs.h>
#include <linux/io.h>
#include <asm/uaccess.h>  /* for put_user */
#include <asm/errno.h>

/* Global variables */
#define BUF_LEN 80            /* Max length of the message from the device */
#define SUCCESS 0
#define DEVICE_NAME "pwm" /* Dev name as it appears in /proc/devices   */
static int Device_Open = 0;  /* Is device open?  Used to prevent multiple access to the device */
static char msg[BUF_LEN];    /* The msg the device will give when asked    */
static char *msg_Ptr;

/* Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
   if (Device_Open) return -EBUSY;

   Device_Open++;
   msg_Ptr = msg;

   return SUCCESS;
}


/* Called when a process closes the device file */
static int device_release(struct inode *inode, struct file *file)
{
   Device_Open --;     /* We're now ready for our next caller */

   return 0;
}

/* Called when a process, which already opened the dev file, attempts to
   read from it.
*/
static ssize_t device_read(struct file *filp,
   char *buffer,    /* The buffer to fill with data */
   size_t length,   /* The length of the buffer     */
   loff_t *offset)  /* Our offset in the file       */
{
   /* Number of bytes actually written to the buffer */
   int bytes_read = 0;

   /* If we're at the end of the message, return 0 signifying end of file */
   if (*msg_Ptr == 0) return 0;

   /* Actually put the data into the buffer */
   while (length && *msg_Ptr)  {

        /* The buffer is in the user data segment, not the kernel segment;
         * assignment won't work.  We have to use put_user which copies data from
         * the kernel data segment to the user data segment. */
         put_user(*(msg_Ptr++), buffer++);

         length--;
         bytes_read++;
   }

   /* Most read functions return the number of bytes put into the buffer */
   return bytes_read;
}


/*  Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t device_write(struct file *filp,
   const char *buff,
   size_t len,
   loff_t *off)
{
   printk ("<1>Sorry, this operation isn't supported.\n");
   return -EINVAL;
}

static struct file_operations Fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};


int __init sysfs_init(void)
{
	int result = 0;

	pwm_obj = kobject_create_and_add(sysfs_dir, kernel_kobj);
	if (pwm_obj == NULL)
	{
		printk (KERN_INFO "%s module failed to load: kobject_create_and_add failed\n", sysfs_file);
		return -ENOMEM;
	}

	result = sysfs_create_group(pwm_obj, &attr_group);
	if (result != 0)
	{
		/* creating files failed, thus we must remove the created directory! */
		printk (KERN_INFO "%s module failed to load: sysfs_create_group failed with result %d\n", sysfs_file, result);
		kobject_put(pwm_obj);
		return -ENOMEM;
	}

	printk(KERN_INFO "/sys/kernel/%s/%s created\n", sysfs_dir, sysfs_file);

   	register_chrdev(DEV_MAJOR, DEV_NAME, &Fops);

	*(unsigned int*)(io_p2v(PWMCLOCK)) = 0x115;
	*(unsigned int*)(io_p2v(LCDCONFIG)) = 0;


	return result;
}

void __exit sysfs_exit(void)
{
   	unregister_chrdev(DEV_MAJOR, DEV_NAME);

	kobject_put(pwm_obj);
	printk (KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);
}

module_init(sysfs_init);
module_exit(sysfs_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("pwm");
