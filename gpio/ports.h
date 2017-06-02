#ifndef PORTS_H
#define PORTS_H

#include "regs.h"

/*
 * The problem is that there are three J-headers which have accessible
 * GPIO pins, but these pins don't match how they are laid out in the
 * registers at all. There's also the problem of there being multiple
 * registers to choose from.
 *
 * We came up with the following - create three arrays which contain all 
 * accessible GPIO pins. This is mapped to an internal bit from the 
 * register they belong to. Since the Y in JX.Y is unique per X, we can
 * then use that to iterate over the array to find. For the protocol
 * we allow the user to access a pin by "JX.Y", so we can choose which
 * port set we want by that first number.
 */

struct PortCombo {
    int PhysicalPin;
    uint32_t RegDIR;
    uint32_t RegOUT;
    uint32_t RegINP;
    int Bit;
    int Jumper;
};

typedef struct PortCombo PortInfo;

static PortInfo Jumper_Pins[] = {
	{ 49, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 8 , 1 },
	{ 50, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 9 , 1 },
	{ 51, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 10, 1 },
	{ 52, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 11, 1 },
	{ 53, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 12, 1 },
	{ 24, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 30, 1 },
	{ 27, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 7 , 1 },
	{ 24, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 1, 2 },
	{ 11, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 2, 2 },
	{ 12, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 3, 2 },
	{ 13, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 4, 2 },
	{ 14, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 5, 2 },
	{ 47, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 0 , 3 },
	{ 56, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 1 , 3 }, 
	{ 48, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 2 , 3 },
	{ 57, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 3 , 3 },
	{ 49, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 4 , 3 },
	{ 58, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 5 , 3 },
	{ 50, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 6 , 3 },
	{ 45, P2_DIR_SET, P2_OUTP_SET, P2_INP_STATE, 1 << 7 , 3 },
	{ 54, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 25, 3 },
	{ 46, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 26, 3 },
	{ 36, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 29, 3 },
	{ 40, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 0, 4 },
	{ 33, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 6, 4 },
};

static PortInfo Jumper_Pins_Read[] = {	
	{ 24, P0_DIR_SET, P0_OUTP_SET, P0_INP_STATE, 1 << 24, 1},
	{ 54, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 10, 3},
	{ 46, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 11, 3},
	{ 36, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 14, 3},
};

static PortInfo getValueFromPortInfo(PortInfo* header, size_t headerSize, int pinToFind, int jumper) {
	PortInfo defaultPort = { 0, 0, 0, 0, 0 };
	int i = 0;
	int items = headerSize;
	for (i = 0; i < items; i++) {
		if (header[i].Jumper == jumper &&
			header[i].PhysicalPin == pinToFind) {
			return header[i];
		}
	}
	return defaultPort;
}

static PortInfo GetJumperPinVal(int jumper, int pin, bool P3_read_input) {
	size_t size;
	if (P3_read_input) {
		size = sizeof(Jumper_Pins_Read)/sizeof(Jumper_Pins_Read[0]);
		return getValueFromPortInfo(Jumper_Pins_Read, size, pin, jumper);
	}
	else {
		size = sizeof(Jumper_Pins)/sizeof(Jumper_Pins[0]);
		return getValueFromPortInfo(Jumper_Pins, size, pin, jumper);
	}
}

#endif