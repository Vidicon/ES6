#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <asm/io.h>
#include <mach/hardware.h>

#define PWM_ENABLE_BIT 0x80000000
#define PWM_DUTY_CYCLE_BITS 0x000000FF
#define PWM_FREQUENCY_BITS 0x0000FF00
#define PWM_DUTY_CYCLE_END_POSITION 8

#define PWM_1_CTRL 0x4005C000

#define MIN_PWM_INPUT 0
#define MAX_PWM_INPUT 100
#define MIN_PWM_OUTPUT 255
#define MAX_PWM_OUTPUT 0

#define MIN_FREQ_INPUT 1
#define MAX_FREQ_INPUT 128
#define MIN_FREQ_OUTPUT 256
#define MAX_FREQ_OUTPUT 0

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


static int __init pwm_init(void)
{
    while(1)
    {
        uint32_t regValue;
        uint32_t PWM1_regValue;
        uint32_t offregValue;
        int mappedValue;
        int inputValue;
        uint32_t mask;

        //PWM1_enable:

        regValue = PWM1_regValue | PWM_ENABLE_BIT;

        iowrite32(regValue,io_p2v(PWM_1_CTRL));


    // PWM1_duty_cycle:
        
        offregValue = PWM1_regValue & ~(PWM_DUTY_CYCLE_BITS);
        mappedValue = map(inputValue, MIN_PWM_INPUT , MAX_PWM_INPUT , MIN_PWM_OUTPUT, MAX_PWM_OUTPUT);

        regValue = offregValue | (mappedValue);
        iowrite32(regValue, io_p2v(PWM_1_CTRL));   
        // PWM1_frequency:
        offregValue = PWM1_regValue & ~(PWM_FREQUENCY_BITS);
        
        mappedValue = map(inputValue, MIN_FREQ_INPUT, MAX_FREQ_INPUT, MIN_FREQ_OUTPUT, MAX_FREQ_OUTPUT);
        mask = mappedValue << PWM_DUTY_CYCLE_END_POSITION;
        regValue = offregValue | (mask);
        iowrite32(regValue, io_p2v(PWM_1_CTRL));   
    } 
    return 0;
}

static void __exit pwm_exit(void)
{
  printk(KERN_INFO "Goodbye, pwm\n");
}

module_init(pwm_init);
module_exit(pwm_exit);




