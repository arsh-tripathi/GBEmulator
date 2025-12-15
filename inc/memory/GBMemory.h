#pragma once

#include <array>
#include <cstdint>

class GBMEM {
    public:
        GBMEM() = default;
        ~GBMEM() = default;
        uint8_t read8(uint16_t address) const { return _MEM[address]; }
        void store8(uint16_t address, uint8_t data) { _MEM[address] = data; }
        uint16_t read16(uint16_t address) const { return (_MEM[address + 1] << 8) + _MEM[address]; }
        void store16(uint16_t address, uint16_t data) { _MEM[address + 1] = data >> 8; _MEM[address] = data & 0xFF; }
    private:
        std::array<uint8_t, 0x10000> _MEM;
};
