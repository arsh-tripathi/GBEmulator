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

uint16_t GBCPU::handleRLCA(GBMEM& mem, uint16_t address) {
    uint8_t data = A();
    uint8_t b7 = data >> 7;
    A((data << 1) + b7);
    set(f_C, b7);
    Log::d("RLCA: RLeft A from " + std::to_string(data) +
           "to " + std::to_string(A()) + " set C to " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRRCA(GBMEM& mem, uint16_t address) {
    uint8_t data = A();
    uint8_t b0 = data & 0b1;
    A((data >> 1) + (b0 << 7));
    set(f_C, b0);
    Log::d("RRCA: RRight A from " + std::to_string(data) +
           "to " + std::to_string(A()) + " set C to " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRLA(GBMEM& mem, uint16_t address) {
    uint8_t data = A();
    uint8_t b7 = data >> 7;
    A((data << 1) + hasC());
    set(f_C, b7);
    Log::d("RLA: RLeft A from " + std::to_string(data) +
           "to " + std::to_string(A()) + " set C to " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRRA(GBMEM& mem, uint16_t address) {
    uint8_t data = A();
    uint8_t b0 = data & 0b1;
    A((data >> 1) + (hasC() << 7));
    set(f_C, b0);
    Log::d("RLA: RLeft A from " + std::to_string(data) +
           "to " + std::to_string(A()) + " set C to " +
           std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleDAA(GBMEM& mem, uint16_t address) {
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

uint16_t GBCPU::handleCPL(GBMEM& mem, uint16_t address) {
    A(~A());
    set(f_N, true);
    set(f_H, true);
    Log::d("CPL: A after " + std::to_string(A()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleSCFA(GBMEM& mem, uint16_t address) {
    set(f_C, true);
    Log::d("SCFA: C " + std::to_string(hasC()), LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleCCF(GBMEM& mem, uint16_t address) {
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

uint16_t GBCPU::handleCPAR8(GBMEM& mem, uint16_t address) {
    Log::d("CPAR8 Instruction", LOG_TAG);
    return address + 1;
}

// ----------------------------
//          BLOCK 3
// ----------------------------
uint16_t GBCPU::handleADDAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("ADDAIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleADCAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("ADCAIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleSUBAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("SUBAIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleSBCAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("SBCAIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleANDAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("ANDAIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleXORAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("XORAIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleORAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("ORAIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleCPAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("CPAIMM8 Instruction", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRETCOND(GBMEM& mem, uint16_t address) {
    Log::d("RETCOND Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRET(GBMEM& mem, uint16_t address) {
    Log::d("RET Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRETI(GBMEM& mem, uint16_t address) {
    Log::d("RETI Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleJPCONDIMM16(GBMEM& mem, uint16_t address) {
    Log::d("JPCONDIMM16 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleJPIMM16(GBMEM& mem, uint16_t address) {
    Log::d("JPIMM16 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleJPHL(GBMEM& mem, uint16_t address) {
    Log::d("JPHL Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleCALLCONDIMM16(GBMEM& mem, uint16_t address) {
    Log::d("CALLCONDIMM16 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleCALLIMM16(GBMEM& mem, uint16_t address) {
    Log::d("CALLIMM16 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRSTTGT3(GBMEM& mem, uint16_t address) {
    Log::d("RSTTGT3 Instruction", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handlePOPR16STK(GBMEM& mem, uint16_t address) {
    Log::d("POPR16STK Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handlePUSHR16STK(GBMEM& mem, uint16_t address) {
    Log::d("PUSHR16STK Instruction", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleLDHCA(GBMEM& mem, uint16_t address) {
    Log::d("LDHCA Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleLDFIMM8A(GBMEM& mem, uint16_t address) {
    Log::d("LDFIMM8A Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleLDIMM16A(GBMEM& mem, uint16_t address) {
    Log::d("LDIMM16A Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleLDHAC(GBMEM& mem, uint16_t address) {
    Log::d("LDHAC Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleLDHAIMM8(GBMEM& mem, uint16_t address) {
    Log::d("LDHAIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleLDAIMM16(GBMEM& mem, uint16_t address) {
    Log::d("LDAIMM16 Instruction", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleADDSPIMM8(GBMEM& mem, uint16_t address) {
    Log::d("ADDSPIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleLDHLSPIMM8(GBMEM& mem, uint16_t address) {
    Log::d("LDHLSPIMM8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleLDSPHL(GBMEM& mem, uint16_t address) {
    Log::d("LDSPHL Instruction", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleDI(GBMEM& mem, uint16_t address) {
    Log::d("DI Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleEI(GBMEM& mem, uint16_t address) {
    Log::d("EI Instruction", LOG_TAG);
    return address + 1;
}

// ----------------------------
//          BLOCK 4
// ----------------------------
uint16_t GBCPU::handleCB(GBMEM& mem, uint16_t address) {
    Log::d("CB Instruction", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleRLCR8(GBMEM& mem, uint16_t address) {
    Log::d("RLCR8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRRCR8(GBMEM& mem, uint16_t address) {
    Log::d("RRCR8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRLR8(GBMEM& mem, uint16_t address) {
    Log::d("RLR8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRRR8(GBMEM& mem, uint16_t address) {
    Log::d("RRR8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleSLAR8(GBMEM& mem, uint16_t address) {
    Log::d("SLAR8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleSRAR8(GBMEM& mem, uint16_t address) {
    Log::d("SRAR8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleSWAPR8(GBMEM& mem, uint16_t address) {
    Log::d("SWAPR8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleSRLR8(GBMEM& mem, uint16_t address) {
    Log::d("SRLR8 Instruction", LOG_TAG);
    return address + 1;
}

uint16_t GBCPU::handleBITB3R8(GBMEM& mem, uint16_t address) {
    Log::d("BITB3R8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleRESB3R8(GBMEM& mem, uint16_t address) {
    Log::d("RESB3R8 Instruction", LOG_TAG);
    return address + 1;
}
uint16_t GBCPU::handleSETB3R8(GBMEM& mem, uint16_t address) {
    Log::d("SETB3R8 Instruction", LOG_TAG);
    return address + 1;
}
