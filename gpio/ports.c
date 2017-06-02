#include "ports.h"
#include <stddef.h>

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
	{ 24, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 24, 1},
	{ 54, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 10, 3},
	{ 46, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 11, 3},
	{ 36, P2_DIR_SET, P3_OUTP_SET, P3_INP_STATE, 1 << 14, 3},
};

PortInfo getValueFromPortInfo(PortInfo* header, size_t headerSize, int pinToFind, int jumper) {
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

PortInfo GetJumperPinVal(int jumper, int pin, bool P3_read_input) {
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
