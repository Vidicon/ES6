#include <stdio.h>
#include <iostream>
#include <cstdint>
#include <cstring>

#include "ports.h"

int doe_dingen(const char *buffer,
            size_t count)
{   
    char command = 'x';
    unsigned int jumper = 0;
    unsigned int pin = 0;
    PortInfo port = { 0, 0, 0 };
    int allBits = 0;

    if (sscanf(buffer, "%c J%d.%d", &command, &jumper, &pin) != 3) {
        printf("Wrong input. Expected format: [i, o] [jumper number] [pin number]");
        return -99;        
    }

    if (jumper < 1 || jumper > 3) {
        printf("Unsupported jumper");
        return -99;
    }

    // A valid bit should be >= 1
    port = GetJumperPinVal(jumper, pin);

    if (port.Bit < 1) { 
        printf("J%d.%d is not supported for GPIO", jumper, pin);
        return -99;
    }
    allBits = 0 | port.Bit; // TODO : Fill existing register

    // TODO : Use base + offset instead of hardcoded to allow for
    // an arbitrary register 
    if (command == 'i') {
        printf("J%d.%d set to INPUT", jumper, pin);
        //memcpy(io_p2v(P2_DIR_CLR),&allBits,sizeof(unsigned int));
    }
    else if (command == 'o') {
        printf("J%d.%d set to OUTPUT", jumper, pin);
        //memcpy(io_p2v(P2_DIR_SET),&allBits,sizeof(unsigned int));
    }
    else {
        printf("Invalid command");
        return -99;
    }

    return count;
}

int main(int argc, char **argv) {
	char command[256];
	printf("Doe dingen: \n");
	std::cin.getline(command, 256);
	doe_dingen(command, 256);
}
