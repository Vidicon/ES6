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

To set a port, we traced the LPC pins to the J headers (see table above),
and we connected a LED to check if our peek/poke command worked.

To set a port:  
* First write P2_DIR_SET to the desired pin (1023-regVal for dir)  
* Then write the same to P2_OUTP_SET (turn on)  
* Then clear the same to P2_OUTP_CLR (turn off)  

# Code uitleg in't engels
-------------------------------------------------------------------------------
iets met code uitleg shizzle
