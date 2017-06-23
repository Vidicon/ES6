#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <mach/hardware.h>
#include <mach/platform.h>
#include <mach/irqs.h>
#include <asm/uaccess.h>

#define DEVICE_NAME 		"adc"
#define ADC_NUMCHANNELS		3

// adc registers
#define	ADCLK_CTRL			io_p2v(0x400040B4)
#define	ADCLK_CTRL1			io_p2v(0x40004060)
#define	ADC_SELECT			io_p2v(0x40048004)
#define	ADC_CTRL			io_p2v(0x40048008)
#define ADC_VALUE			io_p2v(0x40048048)
#define SIC2_ATR			io_p2v(0x40010010)

#define READ_REG(a)			(*(volatile unsigned int *)(a))
#define WRITE_REG(b,a)		(*(volatile unsigned int *)(a) = (b))

// our regs
#define MIC_ATR				io_p2v(0x40008010)
#define SIC1_ER				io_p2v(0x4000C000)
#define SIC2_ER				io_p2v(0x40010000)
#define ADC_INT				IRQ_LPC32XX_TS_IRQ

#define START_ER_INT		io_p2v(0x40004020)
#define TS_INT				1 << 31

#define GPI_1				1 << 23
#define AD_PDN_CTRL 		1 << 2
#define AD_PDN_STROBE 		1 << 1
#define ADC_VALUE_MASK		0x3FF

DECLARE_WAIT_QUEUE_HEAD(adc_handled_event);

static unsigned char	adc_channel = 0;
static int				adc_values[ADC_NUMCHANNELS] = {0, 0, 0};
static bool 			adc_interrupt_handled = false;
static bool 			gpi_interrupt_print = false;

static irqreturn_t      adc_interrupt (int irq, void * dev_id);
static irqreturn_t      gp_interrupt (int irq, void * dev_id);

static void adc_init (void)
{
	unsigned long data;

	data = READ_REG (ADCLK_CTRL);
	data |= 0x1;
	WRITE_REG (data, ADCLK_CTRL);

	data = READ_REG (ADCLK_CTRL1);
	data &= ~0x01ff;
	WRITE_REG (data, ADCLK_CTRL1);

	data = READ_REG(ADC_SELECT);
	data &= ~0x03c0;
	data |=  0x0280;
	WRITE_REG (data, ADC_SELECT);
	
	data = READ_REG(ADC_CTRL);
	data |= AD_PDN_CTRL;
	WRITE_REG (data, ADC_CTRL);

	data = READ_REG(SIC1_ER);
	data |= ADC_INT;
	WRITE_REG (data, SIC1_ER);

	if (request_irq (IRQ_LPC32XX_TS_IRQ, adc_interrupt, IRQF_DISABLED, "adc_interrupt", NULL) != 0)
	{
		printk(KERN_ALERT "ADC IRQ request failed\n");
	}
	
	data = READ_REG(SIC2_ATR);
	data |= GPI_1;
	WRITE_REG (data, SIC2_ATR);

	if (request_irq (IRQ_LPC32XX_GPI_01, gp_interrupt, IRQF_DISABLED, "gpi_interrupt", NULL) != 0)
	{
		printk (KERN_ALERT "GP IRQ request failed\n");
	}
}

static void adc_start (unsigned char channel)
{	
	unsigned long data;
	
	if (channel >= ADC_NUMCHANNELS)
	{
		channel = 0;
	}

	data = READ_REG (ADC_SELECT);

	WRITE_REG((data & ~0x0030) | ((channel << 4) & 0x0030), ADC_SELECT);

	adc_channel = channel;

	data = READ_REG(ADC_CTRL);
	data |= AD_PDN_STROBE;
	WRITE_REG (data, ADC_CTRL);
}

static irqreturn_t adc_interrupt (int irq, void * dev_id)
{
	adc_values[adc_channel] = READ_REG(ADC_VALUE) & ADC_VALUE_MASK;

	if (gpi_interrupt_print) 
	{
		printk(KERN_INFO "ADC(%d)=%d\n", adc_channel, adc_values[adc_channel]);
		adc_channel++;
		if (adc_channel < ADC_NUMCHANNELS)
		{
			adc_start (adc_channel);
		}
		else
		{
			gpi_interrupt_print = false;
		}
	}
	else 
	{
		adc_interrupt_handled = true;
		wake_up_interruptible(&adc_handled_event);
	}

	return (IRQ_HANDLED);
}

static irqreturn_t gp_interrupt(int irq, void * dev_id)
{
	gpi_interrupt_print = true;
	adc_start (0);
	return (IRQ_HANDLED);
}

static void adc_exit (void)
{
	free_irq (IRQ_LPC32XX_GPI_01, NULL);
	free_irq (IRQ_LPC32XX_TS_IRQ, NULL);
}

static ssize_t device_read (struct file * file, char __user *buffer, size_t length, loff_t *f_pos)
{
	int     channel = (int) file->private_data;
	char	return_buffer[128];
	int 	bytesWritten;
	int 	bytesToWrite;

	if (*f_pos > 0) {
		*f_pos = 0;
		return 0;
	}

	if (channel < 0 || channel >= ADC_NUMCHANNELS)
	{
		return -EFAULT;
	}

	adc_interrupt_handled = false;
	adc_start (channel);
	wait_event_interruptible(adc_handled_event, adc_interrupt_handled ); 

	bytesWritten = sprintf(return_buffer, "%d", adc_values[adc_channel]);
	bytesToWrite = copy_to_user(buffer, return_buffer, bytesWritten);
	if (bytesToWrite) {
		bytesWritten = 0;
		printk(KERN_ERR "Failed to write to user");
		return -EFAULT;
	}

	*f_pos = bytesWritten;
	return (bytesWritten);
}

static int device_open (struct inode * inode, struct file * file)
{
	int minor = MINOR(inode->i_rdev);
    file->private_data = (void*)minor;

	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release (struct inode * inode, struct file * file)
{
	module_put(THIS_MODULE);
	return 0;
}


static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.read = device_read,
	.open = device_open,
	.release = device_release
};


static struct chardev
{
	dev_t		dev;
	struct cdev cdev;
} adcdev;

int adcdev_init (void)
{
	int error = alloc_chrdev_region(&adcdev.dev, 0, ADC_NUMCHANNELS, DEVICE_NAME);;
	if(error < 0)
	{
		printk(KERN_WARNING DEVICE_NAME ": dynamic allocation of major number failed, error=%d\n", error);
		return error;
	}

	printk(KERN_INFO DEVICE_NAME ": major number=%d\n", MAJOR(adcdev.dev));

	cdev_init(&adcdev.cdev, &fops);
	adcdev.cdev.owner = THIS_MODULE;
	adcdev.cdev.ops = &fops;

	error = cdev_add(&adcdev.cdev, adcdev.dev, ADC_NUMCHANNELS);
	if(error < 0)
	{
		printk(KERN_WARNING DEVICE_NAME ": unable to add device, error=%d\n", error);
		return error;
	}

	adc_init();
	return 0;
}

void cleanup_module()
{
	cdev_del(&adcdev.cdev);
	unregister_chrdev_region(adcdev.dev, ADC_NUMCHANNELS);
	adc_exit();
}


module_init(adcdev_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("ADC Driver");
