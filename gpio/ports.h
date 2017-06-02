#ifndef PORTS_H
#define PORTS_H

#include "regs.h"
#include <stdbool.h>

struct PortCombo {
    int PhysicalPin;
    unsigned int RegDIR;
    unsigned int RegOUT;
    unsigned int RegINP;
    int Bit;
    int Jumper;
};

typedef struct PortCombo PortInfo;

PortInfo GetJumperPinVal(int jumper, int pin, bool P3_read_input);

#endif
