#ifndef REGS_H
#define REGS_H

#define REG_LCDCONFIG       0x40004054  //= 0
#define VAL_LCDCONFIG       0
#define VAL_MUX             8

/*
 * Due to there being lots of registers to configure and control the
 * GPIO pins, we'll just keep them here. 
 * We did notice a pattern between these register addresses, the 
 * SET, CLR and STATE are separated by 4 always.
 */

#define SET_OFFSET 0
#define CLR_OFFSET 4
#define STATE_OFFSET 8

/*
 * P0 GPIO control registers
 */

#define P0_MUX_SET 	    0x40028120 
#define P0_MUX_CLR 	 	0x40028124 
#define P0_MUX_STATE 	0x40028128 

#define P0_DIR_SET 	 	0x40028050 
#define P0_DIR_CLR 	 	0x40028054 
#define P0_DIR_STATE 	0x40028058 

#define P0_OUTP_SET 	0x40028044 
#define P0_OUTP_CLR 	0x40028048 
#define P0_OUTP_STATE 	0x4002804C 

#define P0_INP_STATE 	0x40028040 

/*
 * P2 GPIO control registers
 */
#define P2_MUX_SET      0x40028028
#define P2_MUX_CLR      0x4002802C
#define P2_MUX_STATE    0x40028030

#define P2_DIR_SET      0x40028010
#define P2_DIR_CLR      0x40028014
#define P2_DIR_STATE    0x40028018

#define P2_OUTP_SET     0x40028020
#define P2_OUTP_CLR     0x40028024

#define P2_INP_STATE    0x4002801C

/*
 * P3 GPIO control registers
 */
#define P3_MUX_SET 	 	0x40028028
#define P3_MUX_CLR 	 	0x40028114
#define P3_MUX_STATE 	0x40028118

// Uses P2 DIR registers

#define P3_OUTP_SET  	0x40028004	
#define P3_OUTP_CLR  	0x40028008	
#define P3_OUTP_STATE 	0x4002800C

#define P3_INP_STATE 	0x40028000

#endif
