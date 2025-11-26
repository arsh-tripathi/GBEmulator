#pragma once

#include <cstdint>
#include <map>
#include <memory/GBMemory.h>
#include <array>

class GBCPU {
      public:
            // REGISTERS
            static const uint8_t z = 1 << 7;
            static const uint8_t n = 1 << 6;
            static const uint8_t h = 1 << 5;
            static const uint8_t c = 1 << 4;

            // GETTERS
            uint8_t A() const { return af & 0xFF00; }
            uint8_t F() const { return af & 0x00FF; }
            uint8_t B() const { return bc & 0xFF00; }
            uint8_t C() const { return bc & 0x00FF; }
            uint8_t D() const { return de & 0xFF00; }
            uint8_t E() const { return de & 0x00FF; }
            uint8_t H() const { return hl & 0xFF00; }
            uint8_t L() const { return hl & 0x00FF; }

            uint16_t AF() const { return af; }
            uint16_t BC() const { return bc; }
            uint16_t DE() const { return de; }
            uint16_t HL() const { return hl; }

            // SETTERS
            void A(const uint8_t & val) { af = af & 0x00FF + val << 8; }
            void F(const uint8_t & val) { af = af & 0xFF00 + val; }
            void B(const uint8_t & val) { bc = bc & 0x00FF + val << 8; }
            void C(const uint8_t & val) { bc = bc & 0xFF00 + val; }
            void D(const uint8_t & val) { de = de & 0x00FF + val << 8; }
            void E(const uint8_t & val) { de = de & 0xFF00 + val; }
            void H(const uint8_t & val) { hl = hl & 0x00FF + val << 8; }
            void L(const uint8_t & val) { hl = hl & 0xFF00 + val; }

            void AF(const uint16_t & val) { af = val; }
            void BC(const uint16_t & val) { bc = val; }
            void DE(const uint16_t & val) { de = val; }
            void HL(const uint16_t & val) { hl = val; }

            bool hasZ() { return af & z; }
            bool hasN() { return af & n; }
            bool hasH() { return af & h; }
            bool hasC() { return af & c; }

            bool setZ() { return af | z; }
            bool setN() { return af | n; }
            bool setH() { return af | h; }
            bool setC() { return af | c; }

            bool unSetZ() { return af & ~z; }
            bool unSetN() { return af & ~n; }
            bool unSetH() { return af & ~h; }
            bool unSetC() { return af & ~c; }

            enum FLAG {
                f_Z, f_N, f_H, f_C
            };

            void set(FLAG f, bool state) {
                switch (f) {
                    case f_Z: 
                        if (state) setZ(); else unSetZ();
                    case f_N: 
                        if (state) setN(); else unSetN();
                    case f_H: 
                        if (state) setH(); else unSetH();
                    case f_C: 
                        if (state) setC(); else unSetC();
                }
            }
          
            uint16_t parseInstruction(GBMEM &mem, uint16_t address);

            #define CBINSTS
            #define OP(a, b) a = b,
            enum InstMask: uint8_t {
                #include<cpu/opcodes.def>
            };
            #undef OP

            #define OP(a, b) a,
            constexpr static std::array<InstMask, 73> instructionList = {
                #include <cpu/opcodes.def>
            };
            #undef OP
            #undef CBINSTS

            using Handler = uint16_t(GBCPU::*)(GBMEM&, uint16_t);
            #define OP(a, b) case a: return &GBCPU::handle##a;
            constexpr static Handler mapInst(InstMask m) {
                switch (m) {
                    #include <cpu/opcodes.def>
                }
            }
            #undef OP

            constexpr static std::array<Handler, 256> makeDecodeTable() {
                std::array<Handler, 256> table{};
                for (int i = 0; i < 256; ++i) {
                    bool mapped = false;
                    for (InstMask mask: instructionList) {
                        if (i & mask == mask) {
                            table[i] = mapInst(mask);
                            mapped = true; 
                            break;
                        }
                    }
                    if (!mapped) table[i] = &GBCPU::handleInvalid;
                }
                return table;
            }

        private:
            uint16_t af, bc, de, hl, SP, PC;

            enum R8 {
                r8_B  = 0,
                r8_C  = 1,
                r8_D  = 2,
                r8_E  = 3,
                r8_H  = 4,
                r8_L  = 5,
                r8_HL = 6,
                r8_A  = 7
            };

            enum R16 {
                r16_BC = 0,
                r16_DE = 1,
                r16_HL = 2,
                r16_SP = 3,
            };

            enum R16STK {
                r16stk_BC = 0,
                r16stk_DE = 1,
                r16stk_HL = 2,
                r16stk_AF = 3,
            };

            enum R16MEM {
                r16mem_BC  = 0,
                r16mem_DE  = 1,
                r16mem_HLP = 2,
                r16mem_HLM = 3,
            };

            enum COND {
                cond_NZ = 0,
                cond_Z  = 1,
                cond_NC = 2,
                cond_C  = 3,
            };

            uint16_t readR16(R16 reg);
            void storeR16(R16 reg, uint16_t val);
            uint8_t readR8(R8 reg);
            void storeR8(R8 reg, uint8_t val);
            bool hasCond(COND cond);

            // INSTRUCTION HANDLERS
          
            uint16_t handleInvalid(GBMEM& mem, uint16_t address);
            // ----------------------------
            //          BLOCK 0
            // ----------------------------
            uint16_t handleNOP(GBMEM& mem, uint16_t address);

            uint16_t handleLDR16IMM16(GBMEM& mem, uint16_t address);
            uint16_t handleLDR16MEMA(GBMEM& mem, uint16_t address);
            uint16_t handleLDAR16MEM(GBMEM& mem, uint16_t address);
            uint16_t handleLDIMM16SP(GBMEM& mem, uint16_t address);

            uint16_t handleINCR16(GBMEM& mem, uint16_t address);
            uint16_t handleDECR16(GBMEM& mem, uint16_t address);
            uint16_t handleADDHLR16(GBMEM& mem, uint16_t address);

            uint16_t handleINCR8(GBMEM& mem, uint16_t address);
            uint16_t handleDECR8(GBMEM& mem, uint16_t address);

            uint16_t handleLDR8IMM8(GBMEM& mem, uint16_t address);

            uint16_t handleRLCA(GBMEM& mem, uint16_t address);
            uint16_t handleRRCA(GBMEM& mem, uint16_t address);
            uint16_t handleRLA(GBMEM& mem, uint16_t address);
            uint16_t handleRRA(GBMEM& mem, uint16_t address);
            uint16_t handleDAA(GBMEM& mem, uint16_t address);
            uint16_t handleCPL(GBMEM& mem, uint16_t address);
            uint16_t handleSCFA(GBMEM& mem, uint16_t address);
            uint16_t handleCCF(GBMEM& mem, uint16_t address);

            uint16_t handleJRIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleJRCONDIMM8(GBMEM& mem, uint16_t address);

            uint16_t handleSTOP(GBMEM& mem, uint16_t address);

            // ----------------------------
            //          BLOCK 1
            // ----------------------------
            uint16_t handleLDR8R8(GBMEM& mem, uint16_t address);

            uint16_t handleHALT(GBMEM& mem, uint16_t address);

            // ----------------------------
            //          BLOCK 2
            // ----------------------------
            uint16_t handleADDAR8(GBMEM& mem, uint16_t address);
            uint16_t handleADCAR8(GBMEM& mem, uint16_t address);
            uint16_t handleSUBAR8(GBMEM& mem, uint16_t address);
            uint16_t handleSBCAR8(GBMEM& mem, uint16_t address);
            uint16_t handleANDAR8(GBMEM& mem, uint16_t address);
            uint16_t handleXORAR8(GBMEM& mem, uint16_t address);
            uint16_t handleCPAR8(GBMEM& mem, uint16_t address);

            // ----------------------------
            //          BLOCK 3
            // ----------------------------
            uint16_t handleADDAIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleADCAIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleSUBAIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleSBCAIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleANDAIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleXORAIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleORAIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleCPAIMM8(GBMEM& mem, uint16_t address);

            uint16_t handleRETCOND(GBMEM& mem, uint16_t address);
            uint16_t handleRET(GBMEM& mem, uint16_t address);
            uint16_t handleRETI(GBMEM& mem, uint16_t address);
            uint16_t handleJPCONDIMM16(GBMEM& mem, uint16_t address);
            uint16_t handleJPIMM16(GBMEM& mem, uint16_t address);
            uint16_t handleJPHL(GBMEM& mem, uint16_t address);
            uint16_t handleCALLCONDIMM16(GBMEM& mem, uint16_t address);
            uint16_t handleCALLIMM16(GBMEM& mem, uint16_t address);
            uint16_t handleRSTTGT3(GBMEM& mem, uint16_t address);

            uint16_t handlePOPR16STK(GBMEM& mem, uint16_t address);
            uint16_t handlePUSHR16STK(GBMEM& mem, uint16_t address);

            uint16_t handleLDHCA(GBMEM& mem, uint16_t address);
            uint16_t handleLDFIMM8A(GBMEM& mem, uint16_t address);
            uint16_t handleLDIMM16A(GBMEM& mem, uint16_t address);
            uint16_t handleLDHAC(GBMEM& mem, uint16_t address);
            uint16_t handleLDHAIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleLDAIMM16(GBMEM& mem, uint16_t address);

            uint16_t handleADDSPIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleLDHLSPIMM8(GBMEM& mem, uint16_t address);
            uint16_t handleLDSPHL(GBMEM& mem, uint16_t address);

            uint16_t handleDI(GBMEM& mem, uint16_t address);
            uint16_t handleEI(GBMEM& mem, uint16_t address);

            // ----------------------------
            //          BLOCK 4
            // ----------------------------
            uint16_t handleCB(GBMEM& mem, uint16_t address);

            uint16_t handleRLCR8(GBMEM& mem, uint16_t address);
            uint16_t handleRRCR8(GBMEM& mem, uint16_t address);
            uint16_t handleRLR8(GBMEM& mem, uint16_t address);
            uint16_t handleRRR8(GBMEM& mem, uint16_t address);
            uint16_t handleSLAR8(GBMEM& mem, uint16_t address);
            uint16_t handleSRAR8(GBMEM& mem, uint16_t address);
            uint16_t handleSWAPR8(GBMEM& mem, uint16_t address);
            uint16_t handleSRLR8(GBMEM& mem, uint16_t address);

            uint16_t handleBITB3R8(GBMEM& mem, uint16_t address);
            uint16_t handleRESB3R8(GBMEM& mem, uint16_t address);
            uint16_t handleSETB3R8(GBMEM& mem, uint16_t address);
};

