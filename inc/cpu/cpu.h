#pragma once

#include <cstdint>

class GBCPU {
  public:
      // REGISTERS
      int16_t af, bc, de, hl, SP, PC;
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

  private:
};
