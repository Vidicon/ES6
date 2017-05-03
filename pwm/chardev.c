#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <mach/hardware.h>

#define REG_PWM1			0x4005C000
#define REG_PWM2			0x4005C004
#define REG_PWMCLOCK		0x400040B8  //= 0x115
#define REG_LCDCONFIG		0x40004054  //= 0

#define MASK_PWM_ENABLE		0x80000000
#define MASK_PWM_DUTY		0x000000FF
#define MASK_PWM_FREQUENCY	0x0000FF00

#define PWM_SHIFT_FREQ		8 // lekker korte naam toch ;p
#define PWM_SHIFT_ENABLE	31

#define VAL_PWMCLOCK		0x115
#define VAL_LCDCONFIG		0

#define PWMCLOCKFREQ		32000

#define DEVICE_NAME			"chardev"
#define DEVICE_MAJOR		103

int minor;
dev_t device;
static int deviceIsOpen = 0; 

static int device_open(struct inode *inode, struct file *file) {
	if (deviceIsOpen) return -EBUSY;
	deviceIsOpen++;

	device = inode->i_rdev;
	minor = MINOR(device);

	printk(KERN_INFO "Major: %d, minor: %d", MAJOR(device), minor);

	return 0;
}

static int device_release(struct inode *inode, struct file *file) {
	deviceIsOpen--;
	return 0;
}

static int map_freq(int pwm_reloadvalue) {
	return (PWMCLOCKFREQ / pwm_reloadvalue) / 256;
}

static int map_duty(int pwm_dutyvalue) {
	return (pwm_dutyvalue * 100) / 256;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset) {
	int PWM1Value = *(unsigned int*)(io_p2v(REG_PWM1));
	int PWM2Value = *(unsigned int*)(io_p2v(REG_PWM2));
	switch(minor) {
		case 0: {
			bool enabled = (PWM1Value & MASK_PWM_ENABLE) != 0;
			printk(KERN_INFO "PWM1 enabled: %s", enabled ? "Yes" : "No");
			break;
		}
		case 1: {
			int reloadValue = (PWM1Value & MASK_PWM_FREQUENCY) >> PWM_SHIFT_FREQ;
			if(reloadValue == 0) {
				printk(KERN_INFO "nope..."); // iets met delen door 0
				break;
			}
			printk(KERN_INFO "PWM1 raw: %d, freq: %dHz", reloadValue, map_freq(reloadValue));
			break;
		}
		case 2: {
			int dutyValue = (PWM1Value & MASK_PWM_DUTY);
			if(dutyValue == 0) {
				printk(KERN_INFO "nope..."); // iets met delen door 0
				break;
			}
			printk(KERN_INFO "PWM1 raw: %d, duty: %d%%", dutyValue, map_duty(dutyValue));
			break;
		}
		case 3: {
			bool enabled = (PWM2Value & MASK_PWM_ENABLE) != 0;
			printk(KERN_INFO "PWM2 enabled: %s", enabled ? "Yes" : "No");
			break;
		}
		case 4: {
			int reloadValue = (PWM2Value & MASK_PWM_FREQUENCY) >> PWM_SHIFT_FREQ;
			if(reloadValue == 0) {
				printk(KERN_INFO "nope..."); // iets met delen door 0
				break;
			}
			printk(KERN_INFO "PWM2 raw: %d, freq: %dHz", reloadValue, map_freq(reloadValue));
			break;
		}
		case 5: {
			int dutyValue = (PWM2Value & MASK_PWM_DUTY);
			if(dutyValue == 0) {
				printk(KERN_INFO "nope..."); // iets met delen door 0
				break;
			}
			printk(KERN_INFO "PWM2 raw: %d, duty: %d%%", dutyValue, map_duty(dutyValue));
			break;
		}
		default:
			printk(KERN_INFO "nope...");
			break;
	}
	return 0;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
	int PWM1Value = *(unsigned int*)(io_p2v(REG_PWM1));
	int PWM2Value = *(unsigned int*)(io_p2v(REG_PWM2));

	int valueToWrite = -1;
	sscanf(buff, "%d", &valueToWrite);

	if(valueToWrite < 0 && valueToWrite > 255) {
		printk(KERN_INFO "nope...");
		return len;
	}
	switch(minor) {
		case 0: {
			*(unsigned int*)(io_p2v(REG_PWM1)) ^= (-valueToWrite ^ PWM1Value) & (1 << PWM_SHIFT_ENABLE);
			printk(KERN_INFO "PWM1 enable written");
			break;
		}
		case 1: {
			int mask = (valueToWrite << PWM_SHIFT_FREQ) & MASK_PWM_FREQUENCY;
			PWM1Value = PWM1Value & ~MASK_PWM_FREQUENCY;
			*(unsigned int*)(io_p2v(REG_PWM1)) = PWM1Value | mask;
			printk(KERN_INFO "PWM1 freq written");
			break;
		}
		case 2: {
			int mask = valueToWrite & MASK_PWM_DUTY;
			PWM1Value = PWM1Value & ~MASK_PWM_DUTY;
			*(unsigned int*)(io_p2v(REG_PWM1)) = PWM1Value | mask;
			printk(KERN_INFO "PWM1 duty written");
			break;
		}
		case 3: {
			*(unsigned int*)(io_p2v(REG_PWM2)) ^= (-valueToWrite ^ PWM2Value) & (1 << PWM_SHIFT_ENABLE);
			printk(KERN_INFO "PWM2 enable written");
			break;
		}
		case 4: {
			int mask = (valueToWrite << PWM_SHIFT_FREQ) & MASK_PWM_FREQUENCY;
			PWM2Value = PWM2Value & ~MASK_PWM_FREQUENCY;
			*(unsigned int*)(io_p2v(REG_PWM2)) = PWM2Value | mask;
			printk(KERN_INFO "PWM2 freq written");
			break;
		}
		case 5: {
			int mask = valueToWrite & MASK_PWM_DUTY;
			PWM2Value = PWM2Value & ~MASK_PWM_DUTY;
			*(unsigned int*)(io_p2v(REG_PWM2)) = PWM2Value | mask;
			printk(KERN_INFO "PWM2 duty written");
			break;
		}
		default:
			printk(KERN_INFO "nope...");
			break;
	}
	return len;
}

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

int init_module(void) {
	int major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);

	if (major < 0) {
		printk ("Registering the character device failed");
		return major;
	}
	printk("Registering the character device succesfull");

	*(unsigned int*)(io_p2v(REG_PWMCLOCK)) = VAL_PWMCLOCK;
	*(unsigned int*)(io_p2v(REG_LCDCONFIG)) = VAL_LCDCONFIG;

	return 0;
}


void cleanup_module(void) {
	unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("pwm");