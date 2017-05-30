GPIO Documentation
===============================================================================
Minh-Triet Diep  
Lars Jaeqx  

## Research

P2_MUX_SET write 8 to set the FOURTH BIT (bit 3 wtf)
Then read P2_MUX_STATE to check this
P2_DIR_CLR write 63 to set all pins as input
P2_DIR_STATE read to check if it's correct
then read P2_INP_STATE to see , reads 1023 when no joy input

poort document source = QVGA_LPC32x0 document

We're reading the following values for the joystick input:

|Joystick  |Register|Bit     |LPC        |J3   |Pins
-----------|--------|--------|-----------|-----|-------------------------
|Nothing   |1023    |x       |x          |     |
|Press     |1022    |0       |P2.22      |J3.47|(9th pin from bottom/left)
|Down      |1007    |4       |P2.27      |J3.49|(8th pin from bottom/left)
|Right     |1015    |3       |P2.26      |J3.57|(4th pin from bottom/left)
|Left      |1021    |1       |P2.23      |J3.56|(5th pin from bottom/right)
|Up        |1019    |2       |P2.25      |J3.48|(9th pin from bottom/right)


## Port mappings:

__PORT 0__  

|Value	|LPC	|SODIMM	|OEM	|J
--------|-------|-------|-------|-------
|1		|P0.0	|X1-88	|P1.20	|J3.32
|2		|P0.1	|X1-36	|P2.11	|J3.8
|4		|P0.2	|X1-31	|P2.6	|J2.11
|8		|P0.3	|X1-32	|P2.7	|J2.12
|16		|P0.4	|X1-33	|P2.8	|J2.13
|32		|P0.5	|X1-34	|P2.9	|J2.14
|64		|P0.6	|X1-90	|P1.22	|J3.33
|128	|P0.7	|X1-91	|P1.23	|J1.27
  
__PORT 1__  
  
|Value	|LPC	|SODIMM	|OEM	|J
--------|-------|-------|-------|-------
|1		|P1.0	|		|		|	
|2		|P1.1	|		|		|		
|4		|P1.2	|		|		|		
|8		|P1.3	|		|		|		
|16		|P1.4	|		|		|		
|32		|P1.5	|		|		|		
|64		|P1.6	|		|		|		
|128	|P1.7	|		|		|		
|256	|P1.8	|		|		|		
|512	|P1.9	|		|		|		
|1024	|P1.10	|		|		|		
|2048	|P1.11	|		|		|		
|4096	|P1.12	|		|		|		
|8192	|P1.13	|		|		|		
|16384	|P1.14	|		|		|		
|32768	|P1.15	|		|		|		
|65536	|P1.16	|		|		|		
|131072	|P1.17	|		|		|		
|262144	|P1.18	|		|		|		
|524288	|P1.19	|		|		|		
|1048576|P1.20	|		|		|		
|2097152|P1.21	|		|		|		
|4194304|P1.22	|		|		|		
|8338608|P1.23	|		|		|		
  
__PORT 2__
The EMC data pins can be used as general purpose GPIO when 16 bit SDRAM or DDRAM is used. Writing a one to bit 3 in the P2_MUX_SET register results in all of the corresponding EMC_D[31:19] pins being configured as GPIO pins P2[12:0].
  
|Value	|LPC	|SODIMM	|OEM	|J
--------|-------|-------|-------|-------
|1		|P2.0	|X1-120 |P2.22	|J3.47
|2		|P2.1	|X1-121 |P2.23	|J3.56
|4		|P2.2	|X1-122 |P2.25	|J3.48
|8		|P2.3	|X1-123 |P2.26	|J3.57
|16		|P2.4	|X1-124 |P2.27	|J3.49
|32		|P2.5	|X1-125 |P2.30	|J3.58
|64		|P2.6	|X1-126 |P2.31	|J3.50
|128	|P2.7	|X1-116 |P2.14	|J3.45
|256	|P2.8	|X1-176 |P3.27	|J1.49
|512	|P2.9	|X1-178 |P3.26	|J1.50
|1024	|P2.10	|X1-180 |P3.25	|J1.51
|2048	|P2.11	|X1-182 |P3.24	|J1.52
|4096	|P2.12	|X1-184 |P3.23	|J1.53
  
|Operation	|Set						|Clear						|State						
------------|---------------------------|---------------------------|----------------------------
|Mux		|0x 4002 8028	P2_MUX_SET	|0x 4002 802C	P2_MUX_CLR	|0x 4002 8030	P2_MUX_STATE
|Direction	|0x 4002 8010	P2_DIR_SET	|0x 4002 8014	P2_DIR_CLR	|0x 4002 8018	P2_DIR_STATE
|Output		|0x 4002 8020	P2_OUTP_SET	|0x 4002 8024	P2_OUTP_CLR	|-
|Input		|-							|-							|0x 4002 801C	P2_INP_STATE
  
Example to set Pin J3.47 on  
8 > 0x4002 8028		P2_MUX_SET  
1 > 0x4002 8010		P2_DIR_SET  
1 > 0x4002 8020		P2_OUTP_SET  
  
__PORT 3__  

|Value	|LPC	|SODIMM	|OEM	|J
--------|-------|-------|-------|-------
|2^25	|GPIO_00|X1-117	|P2.15	|J3.54
|2^26	|GPIO_01|X1-118	|P2.19	|J3.46
|2^27	|GPIO_02|-		|-		|-
|2^28	|GPIO_03|-		|-		|-
|2^29	|GPIO_04|X1-96	|P1.28	|J3.36
|2^30	|GPIO_05|X1-85	|P1.13	|J1.24
  
|Operation	|Set						|Clear						|State						
------------|---------------------------|---------------------------|----------------------------
|Mux		|-							|-							|-
|Direction	|0x 4002 8010	P2_DIR_SET	|0x 4002 8014	P2_DIR_CLR	|0x 4002 8018	P2_DIR_STATE
|Output		|0x 4002 8004	P3_OUTP_SET	|0x 4002 8008	P3_OUTP_CLR	|0x 4002 800C	P3_OUTP_STATE
|Input		|-							|-							|0x 4002 8000	P3_INP_STATE
  
Example to set Pin J3.54
33554432 > 0x4002 8010		P2_DIR_SET  
33554432 > 0x4002 8004		P3_OUTP_SET  

To set a port, we traced the LPC pins to the J headers (see table above),
and we connected a LED to check if our peek/poke command worked.

To set a port:  
* First write P2_DIR_SET to the desired pin (1023-regVal for dir)  
* Then write the same to P2_OUTP_SET (turn on)  
* Then clear the same to P2_OUTP_CLR (turn off)  

# Code uitleg in't engels
-------------------------------------------------------------------------------
iets met code uitleg shizzle
