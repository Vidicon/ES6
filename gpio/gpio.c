#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>


#define P2_MUX_SET      0x40028028  // WO   Write '1' to bit 3 to set EMC_D[31:19] pins being configured as GPIO pins P2[12:0]
#define P2_MUX_CLR      0x4002802C  // WO   Write '1' to clear P2 MUX STATE
#define P2_MUX_STATE    0x40028030  // RO

#define P2_INP_STATE    0x4002801C  // RO
#define P2_OUTP_SET     0x40028020  // WO   
#define P2_OUTP_CLR     0x40028024  // WO
#define P2_DIR_SET      0x40028010  // WO
#define P2_DIR_CLR      0x40028014  // WO
#define P2_DIR_STATE    0x40028018  // RO

/*
    All of this info was gotten from the LPC32x0 data sheet and user manual.

    To get the GPIOs on port 2 operational, the following activities have to be done:
    • the multiplexer for Port 2 (P2_MUX_STATE) must be set such that the pins 31:19 of EMC_D are connected to the GPIO block
    • register P2_DIR_STATE reflects the direction of the pins EMC_D[23:19] (a.k.a. P2.4 .. P2.0) according to your wishes
*/

// To set P2 MUX: 




module_init(sysfs_init);
module_exit(sysfs_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LJ&MT");
MODULE_DESCRIPTION("gpio");
