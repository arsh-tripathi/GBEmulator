#pragma once

#include <array>
#include <cstdint>

class GBMEM {
    public:
        int8_t read8(int16_t address) const { _MEM[address]; }
        void store8(int16_t address, int8_t data) { _MEM[address] = data; }
    private:
        std::array<int8_t, 0x10000> _MEM;
};
