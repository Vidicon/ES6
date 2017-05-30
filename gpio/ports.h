#ifndef PORTS_H
#define PORTS_H
/*
#define J3_47     1
#define J3_56     2
#define J3_48     4
#define J3_57     8
#define J3_49    16
#define J3_58    32
#define J3_50    64
#define J3_45   128
*/

#define P2_BASE_REG 	0x40028010

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
    uint32_t Register;
    int Bit;
};

typedef struct PortCombo PortInfo;

static PortInfo J1_Pins[] = {
	{ 49, P2_BASE_REG, 256 },
	{ 50, P2_BASE_REG, 512 },
	{ 51, P2_BASE_REG, 1024 },
	{ 52, P2_BASE_REG, 2048 },
	{ 53, P2_BASE_REG, 4096 },
};

static PortInfo J2_Pins[] = {
	
};

static PortInfo J3_Pins[] = {
	{ 47, P2_BASE_REG, 1 },
	{ 56, P2_BASE_REG, 2 }, 
	{ 48, P2_BASE_REG, 4 },
	{ 57, P2_BASE_REG, 8 },
	{ 49, P2_BASE_REG, 16 },
	{ 58, P2_BASE_REG, 32 },
	{ 50, P2_BASE_REG, 64 },
	{ 45, P2_BASE_REG, 128 },
};

static PortInfo getValueFromPortInfo(PortInfo* header, int pinToFind) {
	PortInfo defaultPort = { 0, 0, 0 };
	int i = 0;
	int items = sizeof(header)/sizeof(PortInfo);
	for (i = 0; i < items; i++) {
		if (header[i].PhysicalPin == pinToFind) {
			return header[i];
		}
	}
	return defaultPort;
}

static PortInfo GetJumperPinVal(int jumper, int pin) {
	PortInfo defaultPort = { 0, 0, 0 };
	switch(jumper) {
		case 1: {
			return getValueFromPortInfo(J1_Pins, pin);
		}
		case 2: {
			return getValueFromPortInfo(J2_Pins, pin);
		}
		case 3: {
			return getValueFromPortInfo(J3_Pins, pin);
		}
		default:
			return defaultPort;
	}
}

#endif