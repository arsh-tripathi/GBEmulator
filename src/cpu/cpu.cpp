#include <cpu/GBCpu.h>
#include <memory/GBMemory.h>
#include <utils/log.h>
#include <cstdint>
#include <string>

using Handler = uint16_t(GBCPU::*)(GBMEM&, uint16_t);
static constexpr std::array<Handler, 256> decodeTable = GBCPU::makeDecodeTable();
constexpr const char *LOG_TAG = "GBCPU";

uint16_t GBCPU::parseInstruction(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    return (this->*decodeTable[inst])(mem, address);
}

uint16_t GBCPU::handleInvalid(GBMEM& mem, uint16_t address) {
    SDL_Log("[ERROR] [GBCPU] INVALID OPCODE %i RECEIVED AT %i", mem.read8(address), address);
    return address + 1;
}

uint16_t GBCPU::readR16(R16 reg) {
    switch (reg) {
        case r16_BC: return BC();
        case r16_DE: return DE();
        case r16_HL: return HL();
        case r16_SP: return SP;
    }
}

void GBCPU::storeR16(R16 reg, uint16_t val) {
    switch (reg) {
        case r16_BC: BC(val);
        case r16_DE: DE(val);
        case r16_HL: HL(val);
        case r16_SP: SP = val;
    }
}

uint8_t GBCPU::readR8(R8 reg) {
    switch (reg) {
        case r8_A:  return A();
        case r8_B:  return B();
        case r8_C:  return C();
        case r8_D:  return D();
        case r8_E:  return E();
        case r8_H:  return H();
        case r8_L:  return L();
        case r8_HL: return HL();
    }
}

void GBCPU::storeR8(R8 reg, uint8_t val) {
    switch (reg) {
        case r8_A:  A(val);
        case r8_B:  B(val);
        case r8_C:  C(val);
        case r8_D:  D(val);
        case r8_E:  E(val);
        case r8_H:  H(val);
        case r8_L:  L(val);
        case r8_HL: HL(val);
    }
}

bool GBCPU::hasCond(COND cond) {
    switch (cond) {
        case cond_NZ: return !hasZ();
        case cond_Z: return hasZ();
        case cond_NC: return !hasC();
        case cond_C: return hasC();
    }
}

// ----------------------------
//          BLOCK 0
// ----------------------------
uint16_t GBCPU::handleNOP(GBMEM&, uint16_t address) {
    Log::d("NOP Instruction", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleLDR16IMM16(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R16 reg = static_cast<R16>((inst & 0b00110000) >> 4);
    uint16_t val = mem.read16(address + 1);
    storeR16(reg, val);
    Log::d("LDR16IMM16: Load " + std::to_string(val) + 
           " into r16 " + std::to_string(reg), LOG_TAG);
    return address + 3;
}

uint16_t GBCPU::handleLDR16MEMA(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R16 reg = static_cast<R16>((inst & 0b00110000) >> 4);
    uint8_t pointer = readR16(reg);
    uint8_t data = A();
    mem.store8(pointer, data);
    Log::d("LDR16MEMA: Store data " + std::to_string(data) + 
           "(in r8A) to pointer " + std::to_string(pointer), 
           LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleLDAR16MEM(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R16 reg = static_cast<R16>((inst & 0b00110000) >> 4);
    uint8_t pointer = readR16(reg);
    uint8_t data = mem.read8(pointer);
    A(data);
    Log::d("LDAR16MEM: Load data " + std::to_string(data) + 
           "(at " + std::to_string(pointer) + ") to r8 " 
           + std::to_string(r8_A), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleLDIMM16SP(GBMEM& mem, uint16_t address) {
    uint8_t pointer = mem.read16(address + 1);
    mem.store8(pointer, SP & 0xFF);
    mem.store8(pointer + 1, SP >> 8);
    Log::d("LDIMM16SP: Stored " + std::to_string(SP) +
           "(in SP) to pointer" + std::to_string(pointer), 
           LOG_TAG);
    return address + 3;
}

uint16_t GBCPU::handleINCR16(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R16 reg = static_cast<R16>((inst & 0b00110000) >> 4);
    storeR16(reg, readR16(reg) + 1);
    Log::d("INCR16: Incremented r16 " + std::to_string(reg), 
           LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleDECR16(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R16 reg = static_cast<R16>((inst & 0b00110000) >> 4);
    storeR16(reg, readR16(reg) - 1);
    Log::d("DECR16: Decremented r16 " + std::to_string(reg), 
           LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleADDHLR16(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R16 reg = static_cast<R16>((inst & 0b00110000) >> 4);
    uint16_t hl = HL();
    uint16_t r16 = readR16(reg);
    bool overflow11bit = ((hl & 0x0FFF) + (r16 & 0x0FFF)) > 0x0FFF;
    bool overflow15bit = (hl + r16) > 0xFFFF;
    set(f_N, false);
    set(f_H, overflow11bit);
    set(f_C, overflow15bit);
    Log::d("ADDHLR16: Added " + std::to_string(hl) + 
           "(from HL) to " + std::to_string(r16) + 
           "(from " + std::to_string(reg) + ") and "
           " observed overflows: " + 
           std::to_string(overflow11bit) + ", " +
           std::to_string(overflow15bit), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleINCR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>((inst & 0b00111000) >> 3);
    uint8_t r8 = readR8(reg);
    set(f_N, false);
    uint8_t result = r8 + 1;
    bool overflow = ((r8 & 0b111) + 1) >> 3;
    set(f_Z, result == 0);
    set(f_H, overflow);
    storeR8(reg, result);
    Log::d("INCR8: Incremented r8 " + std::to_string(r8) +
           "Z, H: " + std::to_string(result == 0) + ", " +
           std::to_string(overflow), LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleDECR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>((inst & 0b00111000) >> 3);
    uint8_t r8 = readR8(reg);
    set(f_N, true);
    uint8_t result = r8 - 1;
    bool overflow = ((r8 & 0b111) - 1) < 0;
    set(f_Z, result == 0);
    set(f_H, overflow);
    storeR8(reg, result);
    Log::d("DECR8: Decremented r8 " + std::to_string(r8) +
           "Z, H: " + std::to_string(result == 0) + ", " +
           std::to_string(overflow), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleLDR8IMM8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>((inst & 0b00111000) >> 3);
    uint8_t data = mem.read8(address + 1);
    storeR8(reg, data);
    Log::d("LDR8IMM8: Store " + std::to_string(data) +
           " into r8 " + std::to_string(reg), LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleRLCA(GBMEM&, uint16_t address) {
    uint8_t data = A();
    uint8_t b7 = data >> 7;
    A((data << 1) + b7);
    set(f_C, b7);
    Log::d("RLCA: RLeft A from " + std::to_string(data) +
           "to " + std::to_string(A()) + " set C to " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRRCA(GBMEM&, uint16_t address) {
    uint8_t data = A();
    uint8_t b0 = data & 0b1;
    A((data >> 1) + (b0 << 7));
    set(f_C, b0);
    Log::d("RRCA: RRight A from " + std::to_string(data) +
           "to " + std::to_string(A()) + " set C to " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRLA(GBMEM&, uint16_t address) {
    uint8_t data = A();
    uint8_t b7 = data >> 7;
    A((data << 1) + hasC());
    set(f_C, b7);
    Log::d("RLA: RLeft A from " + std::to_string(data) +
           "to " + std::to_string(A()) + " set C to " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRRA(GBMEM&, uint16_t address) {
    uint8_t data = A();
    uint8_t b0 = data & 0b1;
    A((data >> 1) + (hasC() << 7));
    set(f_C, b0);
    Log::d("RLA: RLeft A from " + std::to_string(data) +
           "to " + std::to_string(A()) + " set C to " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleDAA(GBMEM&, uint16_t address) {
    uint16_t adj = 0;
    if (hasN()) {
        if (hasH()) adj += 0x6;
        if (hasC()) adj += 0x60;
        A(A() - adj);
    } else {
        if (hasH() || (A() & 0xF) > 0x9) adj += 0x6;
        if (hasC() || A() > 0x99) {
            adj += 0x60;
            set(f_C, true);
        }
        A(A() + adj);
    }
    Log::d("DAA: A, Z, C: " + std::to_string(A()) +
           ", " + std::to_string(hasZ()) + ", " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleCPL(GBMEM&, uint16_t address) {
    A(~A());
    set(f_N, true);
    set(f_H, true);
    Log::d("CPL: A after " + std::to_string(A()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSCFA(GBMEM&, uint16_t address) {
    set(f_C, true);
    Log::d("SCFA: C " + std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleCCF(GBMEM&, uint16_t address) {
    set(f_C, !hasC());
    Log::d("CCF: C " + std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleJRIMM8(GBMEM& mem, uint16_t address) {
    int8_t offset = mem.read8(address + 1);
    uint16_t resultAdd = address + 1 + offset;
    Log::d("JRIMM8: Jump to " + std::to_string(resultAdd), LOG_TAG);
    return resultAdd;
}

uint16_t GBCPU::handleJRCONDIMM8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    int8_t offset = mem.read8(address + 1);
    COND cond = static_cast<COND>((inst & 0b00011000) >> 3);
    if (hasCond(cond)) {
        uint16_t resultAdd = address + 1 + offset;
        Log::d("JRCONDIMM8: Jump to " + 
               std::to_string(resultAdd), LOG_TAG);
        return resultAdd;
    } else {
        Log::d("JRCONDIMM8: Skip jump", LOG_TAG);
        return address + 2;
    }
}

uint16_t GBCPU::handleSTOP(GBMEM& mem, uint16_t address) {
    // TODO: FIX THIS INSTRUCTION
    Log::d("STOP", LOG_TAG);
    return address + 2;
}

// ----------------------------
//          BLOCK 1
// ----------------------------
uint16_t GBCPU::handleLDR8R8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 dest = static_cast<R8>((inst & 0b00111000) >> 3);
    R8 source = static_cast<R8>(inst & 0b00000111);
    storeR8(dest, readR8(source));
    Log::d("LDR8R8: Stored " + std::to_string(readR8(source))
           + "(from r8" + std::to_string(source) + ") to r8" 
           + std::to_string(dest), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleHALT(GBMEM& mem, uint16_t address) {
    // TODO: Handle this instruction along with STOP
    //       Need to handle interrupts
    Log::d("HALT Instruction", LOG_TAG);
    return address + 1;
}

// ----------------------------
//          BLOCK 2
// ----------------------------
uint16_t GBCPU::handleADDAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b00000111);
    uint8_t a = A();
    uint8_t val = readR8(reg);
    uint8_t result = a + val;
    A(result);
    set(f_Z, result == 0);
    set(f_N, false);
    bool overflow3 = ((a & 0b111) + (val & 0b111)) > 0b111;
    bool overflow7 = ((a & 0b1111111) + (val & 0b1111111)) > 0b1111111;
    set(f_H, overflow3);
    set(f_C, overflow7);
    Log::d("ADDAR8: Add " + std::to_string(val) + "from (r8"
           + std::to_string(reg) + ") to A", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleADCAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b00000111);
    uint8_t a = A();
    uint8_t val = readR8(reg);
    uint8_t carry = hasC();
    uint8_t result = a + val + carry;
    A(result);
    set(f_Z, result == 0);
    set(f_N, false);
    bool overflow3 = ((a & 0b111) + (val & 0b111) + carry) > 0b111;
    bool overflow7 = ((a & 0b1111111) + (val & 0b1111111) + carry) > 0b1111111;
    set(f_H, overflow3);
    set(f_C, overflow7);
    Log::d("ADCAR8: Add " + std::to_string(val) + "from (r8"
           + std::to_string(reg) + ") and C" + 
           std::to_string(carry) + " to A", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSUBAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b00000111);
    uint8_t a = A();
    uint8_t val = readR8(reg);
    uint8_t result = a - val;
    A(result);
    set(f_Z, result == 0);
    set(f_N, false);
    bool borrow3 = ((a & 0b111) - (val & 0b111)) < 0;
    bool borrow7 = ((a & 0b1111111) - (val & 0b1111111)) < 0;
    set(f_H, borrow3);
    set(f_C, borrow7);
    Log::d("SUBAR8: Sub " + std::to_string(val) + "from (r8"
           + std::to_string(reg) + ") to A", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSBCAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b00000111);
    uint8_t a = A();
    uint8_t val = readR8(reg);
    uint8_t carry = hasC();
    uint8_t result = a - val - carry;
    A(result);
    set(f_Z, result == 0);
    set(f_N, false);
    bool borrow3 = ((a & 0b111) - (val & 0b111) - carry) < 0;
    bool borrow7 = ((a & 0b1111111) - (val & 0b1111111) - carry) < 0;
    set(f_H, borrow3);
    set(f_C, borrow7);
    Log::d("SBCAR8: Sub " + std::to_string(val) + "from (r8"
           + std::to_string(reg) + ") and C" + 
           std::to_string(carry) + " to A", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleANDAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b00000111);
    uint8_t a = A();
    uint8_t val = readR8(reg);
    uint8_t result = a & val;
    set(f_Z, result == 0);
    set(f_H, true);
    Log::d("ANDAR8: Set A to the bitwise result of and "
           "with r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleXORAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b00000111);
    uint8_t a = A();
    uint8_t val = readR8(reg);
    uint8_t result = a ^ val;
    set(f_Z, result == 0);
    set(f_H, true);
    Log::d("XORAR8: Set A to the bitwise result of xor "
           "with r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleORAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b00000111);
    uint8_t a = A();
    uint8_t val = readR8(reg);
    uint8_t result = a | val;
    set(f_Z, result == 0);
    set(f_H, true);
    Log::d("ORAR8: Set A to the bitwise result of or "
           "with r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleCPAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b00000111);
    uint8_t a = A();
    uint8_t val = readR8(reg);
    set(f_Z, a - val == 0);
    set(f_N, 1);
    set(f_H, (a & 0xF) < (val & 0xF));
    set(f_C, val > a);
    Log::d("CPAR8: Compare value in A with r8_"
           + std::to_string(reg), LOG_TAG);
    return address + 1;
}

// ----------------------------
//          BLOCK 3
// ----------------------------
uint16_t GBCPU::handleADDAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t a = A();
    uint8_t val = mem.read8(address + 1);
    uint8_t result = a + val;
    A(result);
    set(f_Z, result == 0);
    set(f_N, false);
    bool overflow3 = ((a & 0b111) + (val & 0b111)) > 0b111;
    bool overflow7 = ((a & 0b1111111) + (val & 0b1111111)) > 0b1111111;
    set(f_H, overflow3);
    set(f_C, overflow7);
    Log::d("ADDAIMM8: Add " + std::to_string(val) + " to A", LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleADCAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t a = A();
    uint8_t val = mem.read8(address + 1);
    uint8_t carry = hasC();
    uint8_t result = a + val + carry;
    A(result);
    set(f_Z, result == 0);
    set(f_N, false);
    bool overflow3 = ((a & 0b111) + (val & 0b111) + carry) > 0b111;
    bool overflow7 = ((a & 0b1111111) + (val & 0b1111111) + carry) > 0b1111111;
    set(f_H, overflow3);
    set(f_C, overflow7);
    Log::d("ADCAIMM88: Add " + std::to_string(val)
          + " and C" + std::to_string(carry) + " to A", LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleSUBAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t a = A();
    uint8_t val = mem.read8(address + 1);
    uint8_t result = a - val;
    A(result);
    set(f_Z, result == 0);
    set(f_N, false);
    bool borrow3 = ((a & 0b111) - (val & 0b111)) < 0;
    bool borrow7 = ((a & 0b1111111) - (val & 0b1111111)) < 0;
    set(f_H, borrow3);
    set(f_C, borrow7);
    Log::d("SUBAIMM8: Sub " + std::to_string(val) + " to A", LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleSBCAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t a = A();
    uint8_t val = mem.read8(address + 1);
    uint8_t carry = hasC();
    uint8_t result = a - val - carry;
    A(result);
    set(f_Z, result == 0);
    set(f_N, false);
    bool borrow3 = ((a & 0b111) - (val & 0b111) - carry) < 0;
    bool borrow7 = ((a & 0b1111111) - (val & 0b1111111) - carry) < 0;
    set(f_H, borrow3);
    set(f_C, borrow7);
    Log::d("SBCAIMM8: Sub " + std::to_string(val) + " and C" + 
           std::to_string(carry) + " to A", LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleANDAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t a = A();
    uint8_t val = mem.read8(address + 1);
    uint8_t result = a & val;
    set(f_Z, result == 0);
    set(f_H, true);
    Log::d("ANDAIMM8: Set A to the bitwise result of and "
           "with " + std::to_string(val), LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleXORAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t a = A();
    uint8_t val = mem.read8(address + 1);
    uint8_t result = a ^ val;
    set(f_Z, result == 0);
    set(f_H, true);
    Log::d("XORAIMM8: Set A to the bitwise result of xor "
           "with " + std::to_string(val), LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleORAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t a = A();
    uint8_t val = mem.read8(address + 1);
    uint8_t result = a | val;
    set(f_Z, result == 0);
    set(f_H, true);
    Log::d("ORAIMM8: Set A to the bitwise result of or "
           "with " + std::to_string(val), LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleCPAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t a = A();
    uint8_t val = mem.read8(address + 1);
    set(f_Z, a - val == 0);
    set(f_N, 1);
    set(f_H, (a & 0xF) < (val & 0xF));
    set(f_C, val > a);
    Log::d("CPAIMM8: Compare value in A with r8_"
           + std::to_string(val), LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleRETCOND(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    COND cond = static_cast<COND>((inst & 0b00011000) >> 3);
    if (hasCond(cond)) {
        uint8_t l8 = mem.read8(SP);
        SP += 1;
        uint8_t h8 = mem.read8(SP);
        SP += 1;
        uint16_t ret = (h8 << 8) + l8;
        Log::d("RETCOND: Taken to " + std::to_string(ret), LOG_TAG);
        return ret;
    } else {
        return address + 1;
    }
}

uint16_t GBCPU::handleRET(GBMEM& mem, uint16_t) {
    uint16_t ret = mem.read16(SP);
    SP += 2;
    Log::d("RET: Return to " + std::to_string(ret), LOG_TAG);
    return ret;
}

uint16_t GBCPU::handleRETI(GBMEM& mem, uint16_t) {
    uint16_t ret = mem.read16(SP);
    SP += 2;
    IME_scheduled = 2;
    Log::d("RETI: Return to " + std::to_string(ret), LOG_TAG);
    return ret;
}

uint16_t GBCPU::handleJPCONDIMM16(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    COND cond = static_cast<COND>((inst & 0b00011000) >> 3);
    if (hasCond(cond)) {
        uint16_t nextAdd = mem.read16(address + 1);
        Log::d("JPCONDIMM16: Jump to " + std::to_string(nextAdd), LOG_TAG);
        return nextAdd;
    } else {
        Log::d("JPCONDIMM16: Skip jump", LOG_TAG);
        return address + 3;
    }
}

uint16_t GBCPU::handleJPIMM16(GBMEM& mem, uint16_t address) {
    uint16_t nextAdd = mem.read16(address + 1);
    Log::d("JPIMM16: Jump to " + std::to_string(nextAdd), LOG_TAG);
    return nextAdd;
}

uint16_t GBCPU::handleJPHL(GBMEM&, uint16_t) {
    uint16_t nextAdd = HL();
    Log::d("JPHL: Jump to " + std::to_string(nextAdd), LOG_TAG);
    return nextAdd;
}

uint16_t GBCPU::handleCALLCONDIMM16(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    COND cond = static_cast<COND>((inst & 0b00011000) >> 3);
    if (hasCond(cond)) {
        uint16_t nextInstAdd = address + 3;
        SP -= 2;
        mem.store16(SP, nextInstAdd);
        uint16_t nextAdd = mem.read16(address + 1);
        Log::d("CALLCONDIMM16: Calling " + std::to_string(nextAdd) +
               " from " + std::to_string(nextInstAdd), LOG_TAG);
        return nextAdd;
    } else {
        Log::d("CALLCONDIMM16: Skipping call", LOG_TAG);
        return address + 3;
    }
}

uint16_t GBCPU::handleCALLIMM16(GBMEM& mem, uint16_t address) {
    uint16_t nextInstAdd = address + 3;
    SP -= 2;
    mem.store16(SP, nextInstAdd);
    uint16_t nextAdd = mem.read16(address + 1);
    Log::d("CALLIMM16: Calling " + std::to_string(nextAdd) +
           " from " + std::to_string(nextInstAdd), LOG_TAG);
    return nextAdd;
}

uint16_t GBCPU::handleRSTTGT3(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    uint8_t vecInd = (inst & 0b00111000) >> 3;
    uint16_t nextInstAdd = address + 1;
    SP -= 2;
    uint8_t nextAdd = vec[vecInd];
    mem.store16(SP, nextInstAdd);
    Log::d("RSTTGT3: Calling " + std::to_string(nextAdd) + 
           " from" + std::to_string(nextInstAdd), LOG_TAG);
    return nextAdd;
}

uint16_t GBCPU::handlePOPR16STK(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R16 r16 = static_cast<R16>((inst & 0b00110000) >> 3);
    uint16_t sp = mem.read16(SP);
    storeR16(r16, sp);
    SP += 2;
    Log::d("POPR16STK: Popping " + std::to_string(sp) +
           " ([SP]) into r16" + std::to_string(r16), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handlePUSHR16STK(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R16 r16 = static_cast<R16>((inst & 0b00110000) >> 3);
    uint16_t data = readR16(r16);
    SP -= 2;
    mem.store16(SP, data);
    Log::d("PUSHR16STK: Pushing " + std::to_string(data) +
           " to [SP] from r16" + std::to_string(r16), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleLDHCA(GBMEM& mem, uint16_t address) {
    uint8_t cVal = C();
    uint8_t a = A();
    mem.store8(0xFF00 + cVal, a);
    Log::d("LDHCA: Store data " + std::to_string(a) + 
           " from A to 0xFF00 + " + std::to_string(cVal), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleLDHIMM8A(GBMEM& mem, uint16_t address) {
    uint8_t n8 = mem.read8(address + 1);
    uint8_t a = A();
    mem.store8(0xFF00 + n8, a);
    Log::d("LDHIMM8A: Store data " + std::to_string(a) + 
           " from A to 0xFF00 + " + std::to_string(n8), LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleLDIMM16A(GBMEM& mem, uint16_t address) {
    uint8_t n16 = mem.read16(address + 1);
    uint8_t a = A();
    mem.store8(n16, a);
    Log::d("LDIMM16A: Store data " + std::to_string(a) + 
           " from A to + " + std::to_string(n16), LOG_TAG);
    return address + 3;
}

uint16_t GBCPU::handleLDHAC(GBMEM& mem, uint16_t address) {
    uint8_t cVal = C();
    uint8_t data = mem.read8(0xFF00 + cVal);
    A(data);
    Log::d("LDHAC: Load data " + std::to_string(data) + 
           " to A from 0xFF00 + " + std::to_string(cVal), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleLDHAIMM8(GBMEM& mem, uint16_t address) {
    uint8_t n8 = mem.read8(address + 1);
    uint8_t data = mem.read8(0xFF00 + n8);
    A(data);
    Log::d("LDHAIMM8: Load data " + std::to_string(data) + 
           " to A from 0xFF00 + " + std::to_string(n8), LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleLDAIMM16(GBMEM& mem, uint16_t address) {
    uint8_t n16 = mem.read16(address + 1);
    uint8_t data = mem.read8(n16);
    A(data);
    Log::d("LDAIMM16: Load data " + std::to_string(data) + 
           " to A from 0xFF00 + " + std::to_string(n16), LOG_TAG);
    return address + 3;
}

uint16_t GBCPU::handleADDSPIMM8(GBMEM& mem, uint16_t address) {
    int8_t e8 = static_cast<int8_t>(mem.read8(address + 1));
    SP += e8;
    set(f_Z, 0);
    set(f_N, 0);
    set(f_H, (SP & 0b111) + (e8 & 0b111) > 0b111);
    set(f_H, (SP & 0b1111111) + (e8 & 0b1111111) > 0b1111111);
    Log::d("ADDSPIMM8: Add " + std::to_string(e8) + " to SP", LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleLDHLSPIMM8(GBMEM& mem, uint16_t address) {
    int8_t e8 = static_cast<int8_t>(mem.read8(address + 1));
    SP += e8;
    set(f_Z, 0);
    set(f_N, 0);
    set(f_H, (SP & 0b111) + (e8 & 0b111) > 0b111);
    set(f_H, (SP & 0b1111111) + (e8 & 0b1111111) > 0b1111111);
    HL(SP);
    Log::d("LDHLSPIMM8: Add " + std::to_string(e8) + " to SP", LOG_TAG);
    return address + 2;
}

uint16_t GBCPU::handleLDSPHL(GBMEM&, uint16_t address) {
    SP = HL();
    Log::d("LDSPHL: Load " + std::to_string(HL()) 
           + "into SP", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleDI(GBMEM&, uint16_t address) {
    IME = false;
    IME_scheduled = 0;
    Log::d("DI: Clear IME flag", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleEI(GBMEM&, uint16_t address) {
    IME_scheduled = 1;
    Log::d("EI: Set IME flag after next inst", LOG_TAG);
    return address + 1;
}

// ----------------------------
//          BLOCK 4
// ----------------------------
uint16_t GBCPU::handleCB(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address + 1);
    if ((inst & RLCR8)   == RLCR8)   return handleRLCR8(mem, address + 1);
    if ((inst & RRCR8)   == RRCR8)   return handleRRCR8(mem, address + 1);
    if ((inst & RLR8)    == RLR8)    return handleRLR8(mem, address + 1);
    if ((inst & RRR8)    == RRR8)    return handleRRR8(mem, address + 1);
    if ((inst & SLAR8)   == SLAR8)   return handleSLAR8(mem, address + 1);
    if ((inst & SRAR8)   == SRAR8)   return handleSRAR8(mem, address + 1);
    if ((inst & SWAPR8)  == SWAPR8)  return handleSWAPR8(mem, address + 1);
    if ((inst & SRLR8)   == SRLR8)   return handleSRLR8(mem, address + 1);
    if ((inst & BITB3R8) == BITB3R8) return handleBITB3R8(mem, address + 1);
    if ((inst & RESB3R8) == RESB3R8) return handleRESB3R8(mem, address + 1);
    if ((inst & SETB3R8) == SETB3R8) return handleSETB3R8(mem, address + 1);
    Log::d("CB: Invalid Instruction format", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRLCR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t b7 = r8 >> 7;
    uint8_t result = (r8 << 1) + b7;
    set(f_Z, result == 0);
    set(f_N, false);
    set(f_H, false);
    set(f_C, b7);
    storeR8(reg, result);
    Log::d("RLCR8: Rotate left " + std::to_string(r8) +
           " in r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRRCR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t b0 = r8 & 0b1;
    uint8_t result = (r8 >> 1) + (b0 << 7);
    set(f_Z, result == 0);
    set(f_N, false);
    set(f_H, false);
    set(f_C, b0);
    storeR8(reg, result);
    Log::d("RRCR8: Rotate right " + std::to_string(r8) +
           " in r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRLR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t b7 = r8 >> 7;
    uint8_t result = (r8 << 1) + hasC();
    set(f_Z, result == 0);
    set(f_N, false);
    set(f_H, false);
    set(f_C, b7);
    storeR8(reg, result);
    Log::d("RLR8: Rotate left " + std::to_string(r8) +
           " in r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRRR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t b0 = r8 & 0b1;
    uint8_t result = (r8 >> 1) + (hasC() << 7);
    set(f_Z, result == 0);
    set(f_N, false);
    set(f_H, false);
    set(f_C, b0);
    storeR8(reg, result);
    Log::d("RRR8: Rotate right " + std::to_string(r8) +
           " in r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSLAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t b7 = r8 >> 7;
    uint8_t result = (r8 << 1);
    set(f_Z, result == 0);
    set(f_N, false);
    set(f_H, false);
    set(f_C, b7);
    storeR8(reg, result);
    Log::d("SLAR8: Rotate left " + std::to_string(r8) +
           " in r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSRAR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t b0 = r8 & 0b1;
    uint8_t b7 = r8 & 0b10000000;
    uint8_t result = (r8 >> 1) + b7;
    set(f_Z, result == 0);
    set(f_N, false);
    set(f_H, false);
    set(f_C, b0);
    storeR8(reg, result);
    Log::d("SRAR8: Rotate right " + std::to_string(r8) +
           " in r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSWAPR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    storeR8(reg, ((r8 & 0xF) << 4) + ((r8 & 0xF0) >> 4));
    set(f_Z, r8 == 0);
    set(f_N, 0);
    set(f_H, 0);
    set(f_C, 0);
    Log::d("SWAPR8: Swap bits of r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSRLR8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t b0 = r8 & 0b1;
    uint8_t result = (r8 >> 1);
    set(f_Z, result == 0);
    set(f_N, false);
    set(f_H, false);
    set(f_C, b0);
    storeR8(reg, result);
    Log::d("SRLR8: Rotate right " + std::to_string(r8) +
           " in r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleBITB3R8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    uint8_t bitNum = (inst & 0b00111000) >> 3;
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    set(f_Z, !(r8 & (1 << bitNum)));
    set(f_N, 0);
    set(f_H, 1);
    Log::d("BITB3R8: Check bit num " + std::to_string(bitNum) +
           " of r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRESB3R8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    uint8_t bitNum = (inst & 0b00111000) >> 3;
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t mask = 1 << bitNum;
    storeR8(reg, r8 & (~mask));
    Log::d("RESB3R8: Reset bit num " + std::to_string(bitNum) +
           " of r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSETB3R8(GBMEM& mem, uint16_t address) {
    uint8_t inst = mem.read8(address);
    uint8_t bitNum = (inst & 0b00111000) >> 3;
    R8 reg = static_cast<R8>(inst & 0b111);
    uint8_t r8 = readR8(reg);
    uint8_t mask = 1 << bitNum;
    storeR8(reg, r8 | mask);
    Log::d("SETB3R8: Set bit num " + std::to_string(bitNum) +
           " of r8_" + std::to_string(reg), LOG_TAG);
    return address + 1;
}
