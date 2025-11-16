#pragma once

#include <cstdint>
#include <memory/GBMemory.h>

class GBCPU {
      public:
          // REGISTERS
          static const int8_t Z = 1 << 7;
          static const int8_t n = 1 << 7;
          static const int8_t h = 1 << 7;
          static const int8_t c = 1 << 7;

          // GETTERS
          int8_t A() const { return af & 0xFF00; }
          int8_t F() const { return af & 0x00FF; }
          int8_t B() const { return bc & 0xFF00; }
          int8_t C() const { return bc & 0x00FF; }
          int8_t D() const { return de & 0xFF00; }
          int8_t E() const { return de & 0x00FF; }
          int8_t H() const { return hl & 0xFF00; }
          int8_t L() const { return hl & 0x00FF; }

          int16_t AF() const { return af; }
          int16_t BC() const { return bc; }
          int16_t DE() const { return de; }
          int16_t HL() const { return hl; }

          // SETTERS
          void A(const int8_t & val) { af = af & 0x00FF + val << 8; }
          void F(const int8_t & val) { af = af & 0xFF00 + val; }
          void B(const int8_t & val) { bc = bc & 0x00FF + val << 8; }
          void C(const int8_t & val) { bc = bc & 0xFF00 + val; }
          void D(const int8_t & val) { de = de & 0x00FF + val << 8; }
          void E(const int8_t & val) { de = de & 0xFF00 + val; }
          void H(const int8_t & val) { hl = hl & 0x00FF + val << 8; }
          void L(const int8_t & val) { hl = hl & 0xFF00 + val; }

          void AF(const int16_t & val) { af = val; }
          void BC(const int16_t & val) { bc = val; }
          void DE(const int16_t & val) { de = val; }
          void HL(const int16_t & val) { hl = val; }

          bool hasZ() { return af & Z; }
          bool hasN() { return af & n; }
          bool hasH() { return af & h; }
          bool hasC() { return af & c; }
          
          int16_t parseInstruction(GBMEM &mem, int16_t address);

          enum InstSet {
              
              // ----------------------------
              //          BLOCK 0
              // ----------------------------
              NOP           = 0b00000000,

              LDR16IMM16    = 0b00000001,
              LDR16MEMA     = 0b00000010,
              LDAR16MEM     = 0b00001010,
              LDIMM16SP     = 0b00001000,

              INCR16        = 0b00000011,
              DECR16        = 0b00001011,
              ADDHLR16      = 0b00001001,
              
              INCR8         = 0b00000100,
              DECR8         = 0b00000101,

              LDR8IMM8      = 0b00000110,

              RLCA          = 0b00000111,
              RRCA          = 0b00001111,
              RLA           = 0b00010111,
              RRA           = 0b00011111,
              DAA           = 0b00100111,
              CPL           = 0b00101111,
              SCFA          = 0b00110111,
              CCF           = 0b00111111,

              JRIMM8        = 0b00011000,
              JRCONDIMM8    = 0b00100000,

              STOP          = 0b00010000,

              // ----------------------------
              //          BLOCK 1
              // ----------------------------
              LDR8R8        = 0b01000000,

              HALT          = 0b01110110,

              // ----------------------------
              //          BLOCK 2
              // ----------------------------
              ADDAR8        = 0b10000000,
              ADCAR8        = 0b10001000,
              SUBAR8        = 0b10010000,
              SBCAR8        = 0b10011000,
              ANDAR8        = 0b10100000,
              XORAR8        = 0b10101000,
              CPAR8         = 0b10111000,

              // ----------------------------
              //          BLOCK 3
              // ----------------------------
              ADDAIMM8      = 0b11000110,
              ADCAIMM8      = 0b11001110,
              SUBAIMM8      = 0b11010110,
              SBCAIMM8      = 0b11011110,
              ANDAIMM8      = 0b11100110,
              XORAIMM8      = 0b11101110,
              ORAIMM8       = 0b11110110,
              CPAIMM8       = 0b11111110,

              RETCOND       = 0b11000000,
              RET           = 0b11001001,
              RETI          = 0b11011001,
              JPCONDIMM16   = 0b11000010,
              JPIMM16       = 0b11000011,
              JPHL          = 0b11101001,
              CALLCONDIMM16 = 0b11000100,
              CALLIMM16     = 0b11001101,
              RSTTGT3       = 0b11000111,

              POPR16STK     = 0b11000001,
              PUSHR16STK    = 0b11000101,

              LDHCA         = 0b11100010,
              LDFIMM8A      = 0b11100000,
              LDIMM16A      = 0b11101010,
              LDHAC         = 0b11110010,
              LDHAIMM8      = 0b11110000,
              LDAIMM16      = 0b11111010,

              ADDSPIMM8     = 0b11101000,
              LDHLSPIMM8    = 0b11111000,
              LDSPHL        = 0b11111001,

              DI            = 0b11110011,
              EI            = 0b11111011,

              // ----------------------------
              //          BLOCK 4
              // ----------------------------
              CB            = 0b1101011,

              RLCR8         = 0b00000000,
              RRCR8         = 0b00001000,
              RLR8          = 0b00010000,
              RRR8          = 0b00010000,
              SLAR8         = 0b00100000,
              SRAR8         = 0b00101000,
              SWAPR8        = 0b00110000,
              SRLR8         = 0b00111000,

              BITB3R8       = 0b01000000,
              RESB3R8       = 0b10000000,
              SETB3R8       = 0b11000000
          };

      private:
          int16_t af, bc, de, hl, SP, PC;
};
