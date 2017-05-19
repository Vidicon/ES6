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
struct PortCombo {
    int PortNumber;
    int InternalPin;
};

typedef struct PortCombo PortInfo;

static PortInfo J1_Pins[] = {
	{ 49, 256 },
	{ 50, 512 },
	{ 51, 1024 },
	{ 52, 2048 },
	{ 53, 4096 },
};

static PortInfo J2_Pins[] = {
	
};

static PortInfo J3_Pins[] = {
	{ 47, 1 },
	{ 56, 2 }, 
	{ 48, 4 },
	{ 57, 8 },
	{ 49, 16 },
	{ 58, 32 },
	{ 50, 64 },
	{ 45, 128 },
};

static int getValueFromPortInfo(PortInfo* header, int pinToFind) {
	int i = 0;
	int items = sizeof(header)/sizeof(PortInfo);
	for (i = 0; i < items; i++) {
		if (header[i].PortNumber == pinToFind) {
			return header[i].InternalPin;
		}
	}
	return -1;
}

static int GetJumperPinVal(int jumper, int pin) {
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
			return -1;
	}
}

#endif