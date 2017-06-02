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

#define P0_INP_STATE 	0x40028040 

#define P0_OUTP_SET 	0x40028044 
#define P0_OUTP_CLR 	0x40028048 
#define P0_OUTP_STATE 	0x4002804C 

/*
 * P2 GPIO control registers
 */
// Writing: 0 is input, 1 is output
#define P2_DIR_SET      0x40028010  // WO   Write '1' to set P2.[bit] to Output
#define P2_DIR_CLR      0x40028014  // WO   Write '1' to set P2.[bit] to Input
#define P2_DIR_STATE    0x40028018  // RO   Reads P2.[bit] direction

#define P2_INP_STATE    0x4002801C  // RO   Reads P2.[bit] input state

#define P2_OUTP_SET     0x40028020  // WO   Writes P2.[bit] output state
#define P2_OUTP_CLR     0x40028024  // WO   Write '1' to drive P2.[bit] low

#define P2_MUX_SET      0x40028028  // WO   Write '1' to bit 3 to set EMC_D[31:19] pins being configured as GPIO pins P2[12:0]
#define P2_MUX_CLR      0x4002802C  // WO   Write '1' to clear P2 MUX STATE
#define P2_MUX_STATE    0x40028030  // RO   Is 0 bit



#define P3_MUX_SET 	 	0x40028028
#define P3_MUX_CLR 	 	0x40028114
#define P3_MUX_STATE 	0x40028118

#define P3_OUTP_SET  	0x40028004	
#define P3_OUTP_CLR  	0x40028008	
#define P3_OUTP_STATE 	0x4002800C
#define P3_INP_STATE 	0x40028000









#endif