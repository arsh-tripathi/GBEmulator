#include <GBMemory.h>
#include <stdexcept>


uint8_t GBMEM::read8(uint16_t address) const {
    if (isOAM(address) && _OAM_locked) return 0xFF;
    if (isVRAM(address) && _VRAM_locked) return 0xFF;
    if (isPalletes(address) && _PAL_locked) return 0xFF;
    return _MEM[address];
}

void GBMEM::store8(uint16_t address, uint8_t data) {
    if (isOAM(address) && _OAM_locked) return;
    if (isVRAM(address) && _VRAM_locked) return;
    if (isPalletes(address) && _PAL_locked) return;
    _MEM[address] = data;
}

uint16_t GBMEM::read16(uint16_t address) const {
    if (isOAM(address) && _OAM_locked) return 0xFF;
    if (isVRAM(address) && _VRAM_locked) return 0xFF;
    if (isPalletes(address) && _PAL_locked) return 0xFF;
    return (_MEM[address + 1] << 8) + _MEM[address];
}

void GBMEM::store16(uint16_t address, uint16_t data) {
    if (isOAM(address) && _OAM_locked) return;
    if (isVRAM(address) && _VRAM_locked) return;
    if (isPalletes(address) && _PAL_locked) return;
    _MEM[address + 1] = data >> 8; _MEM[address] = data & 0xFF;
}

bool GBMEM::isOAM(uint16_t address) const {
    return 0xFE00 <= address && address <= 0xFE9F;
}

bool GBMEM::isVRAM(uint16_t address) const {
    return 0x8000 <= address && address <= 0x9FFF;
}

bool GBMEM::isPalletes(uint16_t address) const {
    return 0xFF47 <= address && address <= 0xFF49;
}


void GBMEM::lockOAM() {
    if (_OAM_locked)
        throw std::logic_error("Relocking OAM");
    _OAM_locked = true;
}

void GBMEM::unlockOAM() {
    if (!_OAM_locked)
        throw std::logic_error("Reunlocking OAM");
    _OAM_locked = false;
}

void GBMEM::lockVRAM() {
    if (_VRAM_locked)
        throw std::logic_error("Relocking VRAM");
    _VRAM_locked = true;
}

void GBMEM::unlockVRAM() {
    if (!_VRAM_locked)
        throw std::logic_error("Reunlocking VRAM");
    _VRAM_locked = false;
}

void GBMEM::lockPallettes() {
    if (_PAL_locked)
        throw std::logic_error("Relocking PAL");
    _PAL_locked = true;
}

void GBMEM::unlockPallettes() {
    if (!_PAL_locked)
        throw std::logic_error("Reunlocking PAL");
    _PAL_locked = false;
}
