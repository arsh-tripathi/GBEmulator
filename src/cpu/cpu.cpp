#include <cpu/GBCpu.h>
#include <memory/GBMemory.h>

int16_t GBCPU::parseInstruction(GBMEM& mem, int16_t address) {
    int8_t inst = mem.read8(address);
    int16_t next = address + 0x8;



    return next;
}
