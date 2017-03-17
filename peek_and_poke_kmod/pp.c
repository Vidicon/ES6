/*  
 *  hello-2.c - Demonstrating the module_init() and module_exit() macros.
 *  This is preferred over using init_module() and cleanup_module().
 */
#include <linux/module.h>     /* Needed by all modules */
#include <linux/kernel.h>     /* Needed for KERN_INFO */
#include <linux/init.h>       /* Needed for the macros */
#include <mach/hardware.h>

#define RTC_UCOUNT 0x40024000 
#define RTC_DCOUNT RTC_UCOUNT+4

static int __init rtc_init(void)
{
  printk(KERN_INFO "RTC_U: %u", *(unsigned int*)(io_p2v(RTC_UCOUNT)));
  return 0;
}

static void __exit rtc_exit(void)
{
  printk(KERN_INFO "RIP\n");
}

module_init(rtc_init);
module_exit(rtc_exit);
