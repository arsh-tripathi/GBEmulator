#include <gtest/gtest.h>
#include <cpu/GBCpu.h>
#include <memory/GBMemory.h>

#define SKIP GTEST_SKIP() << "SKIPPING: Incomplete test"

// ----------------------------
//          BLOCK 0
// ----------------------------
TEST(CPUTest, NOOP) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00000000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
}

TEST(CPUTest, LDR16N16) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00000001);
    mem.store16(0x1, 0x5678);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x3);
    ASSERT_EQ(res.second, 3);
    ASSERT_EQ(cpu.BC(), 0x5678);
}

TEST(CPUTest, LDR16MEMA) {
    GBCPU cpu;
    GBMEM mem;
    cpu.BC(0xF);
    cpu.A(0x1);
    mem.store8(0x0, 0b00000010);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(mem.read8(0xF), 0x1);
}

TEST(CPUTest, LDAR16MEM) {
    GBCPU cpu;
    GBMEM mem;
    cpu.BC(0xF);
    mem.store8(0x0, 0b00001010);
    mem.store8(0xF, 0x1);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0x1);
}

TEST(CPUTest, LDIMM16SP) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xABCD);
    mem.store8(0x0, 0b00001000);
    mem.store16(0x1, 0xFEDC);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x3);
    ASSERT_EQ(res.second, 5);
    ASSERT_EQ(mem.read8(0xFEDC), 0xCD);
    ASSERT_EQ(mem.read8(0xFEDD), 0xAB);
}

// OP(INCR16        , 0b00000011, 0b11001111)
TEST(CPUTest, INCR16) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00000011);
    cpu.BC(0x1);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.BC(), 0x2);
}

// OP(DECR16        , 0b00001011, 0b11001111)
TEST(CPUTest, DECR16) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00001011);
    cpu.BC(0x1);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.BC(), 0x0);
}

// OP(ADDHLR16      , 0b00001001, 0b11001111)
TEST(CPUTest, ADDHLR16) {
    GBCPU cpu;
    GBMEM mem;
    cpu.HL(0x0FFF);
    cpu.BC(0x0001);
    mem.store8(0x0, 0b00001001);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.HL(), 0x1000);
    ASSERT_TRUE(cpu.hasH());
    ASSERT_FALSE(cpu.hasN());
    ASSERT_FALSE(cpu.hasC());
}

TEST(CPUTest, ADDHLR16_OF) {
    GBCPU cpu;
    GBMEM mem;
    cpu.HL(0xFFFF);
    cpu.BC(0x0001);
    mem.store8(0x0, 0b00001001);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.HL(), 0x0000);
    ASSERT_TRUE(cpu.hasH());
    ASSERT_FALSE(cpu.hasN());
    ASSERT_TRUE(cpu.hasC());
}
//
// OP(INCR8         , 0b00000100, 0b11000111)
TEST(CPUTest, INCR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x0F);
    mem.store8(0x0, 0b00000100);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.B(), 0x10);
    ASSERT_TRUE(cpu.hasH());
    ASSERT_FALSE(cpu.hasN());
    ASSERT_FALSE(cpu.hasZ());
}

TEST(CPUTest, INCR8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0xFF);
    mem.store8(0x0, 0b00000100);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.B(), 0x00);
    ASSERT_TRUE(cpu.hasH());
    ASSERT_FALSE(cpu.hasN());
    ASSERT_TRUE(cpu.hasZ());
}

// OP(DECR8         , 0b00000101, 0b11000111)
TEST(CPUTest, DECR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x10);
    mem.store8(0x0, 0b00000101);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.B(), 0x0F);
    ASSERT_TRUE(cpu.hasH());
    ASSERT_TRUE(cpu.hasN());
    ASSERT_FALSE(cpu.hasZ());
}

TEST(CPUTest, DECR8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x01);
    mem.store8(0x0, 0b00000101);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.B(), 0x00);
    ASSERT_FALSE(cpu.hasH());
    ASSERT_TRUE(cpu.hasN());
    ASSERT_TRUE(cpu.hasZ());
}

//
// OP(LDR8IMM8      , 0b00000110, 0b11000111)
TEST(CPUTest, LDR8IMM8) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00000110);
    mem.store8(0x1, 0x1);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0x1);
}

// OP(RLCA          , 0b00000111, 0b11111111)
TEST(CPUTest, RLCA) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b10101010);
    mem.store8(0x0, 0b00000111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b01010101);
    ASSERT_TRUE(cpu.hasC());
}

TEST(CPUTest, RLCA_C0) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b01010101);
    mem.store8(0x0, 0b00000111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b10101010);
    ASSERT_FALSE(cpu.hasC());
}

// OP(RRCA          , 0b00001111, 0b11111111)
TEST(CPUTest, RRCA) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b01010101);
    mem.store8(0x0, 0b00001111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b10101010);
    ASSERT_TRUE(cpu.hasC());
}

TEST(CPUTest, RRCA_C0) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b10101010);
    mem.store8(0x0, 0b00001111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b01010101);
    ASSERT_FALSE(cpu.hasC());
}

// OP(RLA           , 0b00010111, 0b11111111)
TEST(CPUTest, RLA) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b10101010);
    mem.store8(0x0, 0b00010111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b01010100);
    ASSERT_TRUE(cpu.hasC());
}

TEST(CPUTest, RLA_C0) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b01010101);
    cpu.setC();
    mem.store8(0x0, 0b00010111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b10101011);
    ASSERT_FALSE(cpu.hasC());
}

// OP(RRA           , 0b00011111, 0b11111111)
TEST(CPUTest, RRA) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b01010101);
    mem.store8(0x0, 0b00011111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b00101010);
    ASSERT_TRUE(cpu.hasC());
}

TEST(CPUTest, RRA_C0) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b10101010);
    cpu.setC();
    mem.store8(0x0, 0b00011111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b11010101);
    ASSERT_FALSE(cpu.hasC());
}

// OP(DAA           , 0b00100111, 0b11111111)
TEST(CPUTest, DAA_NnHnC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setN();
    cpu.A(0x1);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x1);
    ASSERT_FALSE(cpu.hasH());
}

TEST(CPUTest, DAA_NHnC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setN();
    cpu.setH();
    cpu.A(0x6);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x0);
    ASSERT_TRUE(cpu.hasZ());
}

TEST(CPUTest, DAA_NnHC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setN();
    cpu.setC();
    cpu.A(0x61);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x1);
}

TEST(CPUTest, DAA_NHC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setN();
    cpu.setH();
    cpu.setC();
    cpu.A(0x67);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x1);
}

TEST(CPUTest, DAA_nNnHnC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x1);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x1);
}

TEST(CPUTest, DAA_nNHnC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setH();
    cpu.A(0x00);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x6);
}

TEST(CPUTest, DAA_nNH2nC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0xA);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x10);
}

TEST(CPUTest, DAA_nNnHC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    cpu.A(0x1);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x61);
    ASSERT_TRUE(cpu.hasC());
}

TEST(CPUTest, DAA_nNnHC2) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0xA0);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x0);
    ASSERT_TRUE(cpu.hasZ());
    ASSERT_TRUE(cpu.hasC());
}

TEST(CPUTest, DAA_nNHC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setH();
    cpu.setC();
    cpu.A(0x1);
    mem.store8(0x0, 0b00100111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x67);
}

// OP(CPL           , 0b00101111, 0b11111111)
TEST(CPUTest, CPL) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b10000001);
    mem.store8(0x0, 0b00101111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b01111110);
    ASSERT_TRUE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
}

// OP(SCFA          , 0b00110111, 0b11111111)
TEST(CPUTest, SCFA) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00110111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
}

// OP(CCF           , 0b00111111, 0b11111111)
TEST(CPUTest, CCF_C1) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00111111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_TRUE(cpu.hasC());
}

TEST(CPUTest, CCF_C0) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00111111);
    cpu.setC();
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_FALSE(cpu.hasC());
}

//
// OP(JRIMM8        , 0b00011000, 0b11111111)
TEST(CPUTest, JRIMM8_PO) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x2, 0b00011000);
    mem.store8(0x3, 0b00000100);
    auto res = cpu.parseInstruction(mem, 0x2);
    ASSERT_EQ(res.first, 0x8);
    ASSERT_EQ(res.second, 3);
}

TEST(CPUTest, JRIMM8_NO) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x2, 0b00011000);
    mem.store8(0x3, 0b11111110); // -2
    auto res = cpu.parseInstruction(mem, 0x2);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 3);
}

// OP(JRCONDIMM8    , 0b00100000, 0b11100111)
TEST(CPUTest, JRCONDIMM8_T) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    mem.store8(0x0, 0b00111000);
    mem.store8(0x1, 0x3);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x5);
    ASSERT_EQ(res.second, 3);
}

TEST(CPUTest, JRCONDIMM8_NT) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00101000);
    mem.store8(0x1, 0x3);
    ASSERT_FALSE(cpu.hasZ());
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
}

//
// OP(STOP          , 0b00010000, 0b11111111)
TEST(CPUTest, STOP) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b00010000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 0);
    SKIP;
}

//
// // ----------------------------
// //          BLOCK 1
// // ----------------------------
// OP(LDR8R8        , 0b01000000, 0b11000000)
TEST(CPUTest, LDR8R8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.C(0x1);
    mem.store8(0x0, 0b01000001);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.B(), 0x1);
}
//
// OP(HALT          , 0b01110110, 0b11111111)
TEST(CPUTest, HALT) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b01110110);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 0);
    SKIP;
}
//
// // ----------------------------
// //          BLOCK 2
// // ----------------------------
// OP(ADDAR8        , 0b10000000, 0b11111000)
TEST(CPUTest, ADDAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0xFF);
    cpu.B(0x1);
    mem.store8(0x0, 0b10000000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x00);
    ASSERT_TRUE(cpu.hasZ());
    ASSERT_TRUE(cpu.hasH());
    ASSERT_TRUE(cpu.hasC());
}

// OP(ADCAR8        , 0b10001000, 0b11111000)
TEST(CPUTest, ADCAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0xFE);
    cpu.B(0x1);
    cpu.setC();
    mem.store8(0x0, 0b10001000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x00);
    ASSERT_TRUE(cpu.hasZ());
    ASSERT_TRUE(cpu.hasH());
    ASSERT_TRUE(cpu.hasC());
}

// OP(SUBAR8        , 0b10010000, 0b11111000)
TEST(CPUTest, SUBAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x0F);
    cpu.A(0x10);
    mem.store8(0x0, 0b10010000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x1);
    ASSERT_TRUE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
}

TEST(CPUTest, SUBAR8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x10);
    cpu.A(0x10);
    mem.store8(0x0, 0b10010000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x0);
    ASSERT_TRUE(cpu.hasZ());
}

TEST(CPUTest, SUBAR8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x10);
    cpu.A(0x0F);
    mem.store8(0x0, 0b10010000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0xFF);
    ASSERT_TRUE(cpu.hasC());
}

// OP(SBCAR8        , 0b10011000, 0b11111000)
TEST(CPUTest, SBCAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x0E);
    cpu.setC();
    cpu.A(0x10);
    mem.store8(0x0, 0b10011000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x1);
    ASSERT_TRUE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
}

TEST(CPUTest, SBCAR8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x0F);
    cpu.setC();
    cpu.A(0x10);
    mem.store8(0x0, 0b10011000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x0);
    ASSERT_TRUE(cpu.hasZ());
}

TEST(CPUTest, SBCAR8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x10);
    cpu.A(0x0F);
    mem.store8(0x0, 0b10011000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0xFF);
    ASSERT_TRUE(cpu.hasC());
}

// OP(ANDAR8        , 0b10100000, 0b11111000)
TEST(CPUTest, ANDAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b10101100);
    cpu.B(0b11001010);
    mem.store8(0x0, 0b10100000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b10001000);
    ASSERT_TRUE(cpu.hasH());
}

TEST(CPUTest, ANDAR8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b10101100);
    cpu.B(0b01010011);
    mem.store8(0x0, 0b10100000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0);
    ASSERT_TRUE(cpu.hasZ());
}

// OP(XORAR8        , 0b10101000, 0b11111000)
TEST(CPUTest, XORAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b11110000);
    cpu.B(0b11001100);
    mem.store8(0x0, 0b10101000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b00111100);
}

TEST(CPUTest, XORAR8_Z) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b10101000);
    cpu.A(0b11110000);
    cpu.B(0b11110000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0);
    ASSERT_TRUE(cpu.hasZ());
}

// OP(ORAR8         , 0b10110000, 0b11111000)
TEST(CPUTest, ORAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b11110000);
    cpu.B(0b11001100);
    mem.store8(0x0, 0b10110000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0b11111100);
}

TEST(CPUTest, ORAR8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b00000000);
    cpu.B(0b00000000);
    mem.store8(0x0, 0b10110000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0);
    ASSERT_TRUE(cpu.hasZ());
}

// OP(CPAR8         , 0b10111000, 0b11111000)
TEST(CPUTest, CPAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x0F);
    cpu.A(0x10);
    mem.store8(0x0, 0b10111000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_EQ(cpu.A(), 0x10);
    ASSERT_TRUE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
}

TEST(CPUTest, CPAR8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x10);
    cpu.A(0x10);
    mem.store8(0x0, 0b10111000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_TRUE(cpu.hasZ());
}

TEST(CPUTest, CPAR8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0x10);
    cpu.A(0x0F);
    mem.store8(0x0, 0b10111000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    ASSERT_TRUE(cpu.hasC());
}

//
// // ----------------------------
// //          BLOCK 3
// // ----------------------------
// OP(ADDAIMM8      , 0b11000110, 0b11111111)
TEST(CPUTest, ADDAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0xFF);
    mem.store8(0x0, 0b11000110);
    mem.store8(0x1, 1);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0);
    ASSERT_TRUE(cpu.hasZ());
    ASSERT_FALSE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
    ASSERT_TRUE(cpu.hasC());
}

// OP(ADCAIMM8      , 0b11001110, 0b11111111)
TEST(CPUTest, ADCAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    cpu.A(0xFE);
    mem.store8(0x0, 0b11001110);
    mem.store8(0x1, 1);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0);
    ASSERT_TRUE(cpu.hasZ());
    ASSERT_FALSE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
    ASSERT_TRUE(cpu.hasC());
}

// OP(SUBAIMM8      , 0b11010110, 0b11111111)
TEST(CPUTest, SUBAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x10);
    mem.store8(0x0, 0b11010110);
    mem.store8(0x1, 0x0F);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0x1);
    ASSERT_TRUE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
}

TEST(CPUTest, SUBAIMM8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x10);
    mem.store8(0x0, 0b11010110);
    mem.store8(0x1, 0x10);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0x0);
    ASSERT_TRUE(cpu.hasZ());
}

TEST(CPUTest, SUBAIMM8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x0F);
    mem.store8(0x0, 0b11010110);
    mem.store8(0x1, 0x10);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0xFF);
    ASSERT_TRUE(cpu.hasC());
}

// OP(SBCAIMM8      , 0b11011110, 0b11111111)
TEST(CPUTest, SBCAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x10);
    cpu.setC();
    mem.store8(0x0, 0b11011110);
    mem.store8(0x1, 0xE);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0x1);
    ASSERT_TRUE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
}

TEST(CPUTest, SBCAIMM8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x10);
    cpu.setC();
    mem.store8(0x0, 0b11011110);
    mem.store8(0x1, 0xF);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0x0);
    ASSERT_TRUE(cpu.hasZ());
}

TEST(CPUTest, SBCAIMM8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x0F);
    mem.store8(0x0, 0b11011110);
    mem.store8(0x1, 0x10);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0xFF);
    ASSERT_TRUE(cpu.hasC());
}

// OP(ANDAIMM8      , 0b11100110, 0b11111111)
TEST(CPUTest, ANDAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b11110000);
    mem.store8(0x0, 0b11100110);
    mem.store8(0x1, 0b11001100);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0b11000000);
    ASSERT_TRUE(cpu.hasH());
}

TEST(CPUTest, ANDAIMM8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b00000000);
    mem.store8(0x0, 0b11100110);
    mem.store8(0x1, 0b11111111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0);
    ASSERT_TRUE(cpu.hasZ());
}

// OP(XORAIMM8      , 0b11101110, 0b11111111)
TEST(CPUTest, XORAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b11110000);
    mem.store8(0x0, 0b11101110);
    mem.store8(0x1, 0b11001100);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0b00111100);
}

TEST(CPUTest, XORAIMM8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b11111111);
    mem.store8(0x0, 0b11101110);
    mem.store8(0x1, 0b11111111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0);
    ASSERT_TRUE(cpu.hasZ());
}

// OP(ORAIMM8       , 0b11110110, 0b11111111)
TEST(CPUTest, ORAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b11110000);
    mem.store8(0x0, 0b11110110);
    mem.store8(0x1, 0b11001100);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0b11111100);
}

TEST(CPUTest, ORAIMM8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0b00000000);
    mem.store8(0x0, 0b11110110);
    mem.store8(0x1, 0b00000000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0);
    ASSERT_TRUE(cpu.hasZ());
}

// OP(CPAIMM8       , 0b11111110, 0b11111111)
TEST(CPUTest, CPAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x10);
    mem.store8(0x0, 0b11111110);
    mem.store8(0x1, 0x0F);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0x10);
    ASSERT_TRUE(cpu.hasN());
    ASSERT_TRUE(cpu.hasH());
}

TEST(CPUTest, CPAIMM8_Z) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x10);
    mem.store8(0x0, 0b11111110);
    mem.store8(0x1, 0x10);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_TRUE(cpu.hasZ());
}

TEST(CPUTest, CPAIMM8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0x0F);
    mem.store8(0x0, 0b11111110);
    mem.store8(0x1, 0x10);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_TRUE(cpu.hasC());
}

//
// OP(RETCOND       , 0b11000000, 0b11100111)
TEST(CPUTest, RETCOND_T) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    cpu.SP(0xFEDC);
    mem.store16(0xFEDC, 0xABCD);
    mem.store8(0x0, 0b11011000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0xABCD);
    ASSERT_EQ(res.second, 5);
    ASSERT_EQ(cpu.SP(), 0xFEDE);
}

TEST(CPUTest, RETCOND_NT) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store16(0xFEDC, 0xABCD);
    mem.store8(0x0, 0b11011000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.SP(), 0xFEDC);
}

// OP(RET           , 0b11001001, 0b11111111)
TEST(CPUTest, RET) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store16(0xFEDC, 0xABCD);
    mem.store8(0x0, 0b11001001);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0xABCD);
    ASSERT_EQ(res.second, 4);
    ASSERT_EQ(cpu.SP(), 0xFEDE);
}

// OP(RETI          , 0b11011001, 0b11111111)
TEST(CPUTest, RETI) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store16(0xFEDC, 0xABCD);
    mem.store8(0x0, 0b11011001);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0xABCD);
    ASSERT_EQ(res.second, 4);
    ASSERT_EQ(cpu.SP(), 0xFEDE);
    SKIP;
}

// OP(JPCONDIMM16   , 0b11000010, 0b11100111)
TEST(CPUTest, JPCONDIMM16_T) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    mem.store8(0x0, 0b11011010);
    mem.store16(0x1, 0x10FE);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x10FE);
    ASSERT_EQ(res.second, 4);
}

TEST(CPUTest, JPCONDIMM16_NT) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b11011010);
    mem.store16(0x1, 0x10FE);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x3);
    ASSERT_EQ(res.second, 3);
}

// OP(JPIMM16       , 0b11000011, 0b11111111)
TEST(CPUTest, JPIMM16) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b11000011);
    mem.store16(0x1, 0x10FE);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x10FE);
    ASSERT_EQ(res.second, 4);
}

// OP(JPHL          , 0b11101001, 0b11111111)
TEST(CPUTest, JPHL) {
    GBCPU cpu;
    GBMEM mem;
    cpu.HL(0x10FE);
    mem.store8(0x0, 0b11101001);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x10FE);
    ASSERT_EQ(res.second, 1);
}

// OP(CALLCONDIMM16 , 0b11000100, 0b11100111)
TEST(CPUTest, CALLCONDIMM16_T) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    cpu.SP(0xFEDC);
    mem.store8(0x0, 0b11011100);
    mem.store16(0x1, 0x10FE);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x10FE);
    ASSERT_EQ(res.second, 6);
    ASSERT_EQ(cpu.SP(), 0xFEDA);
    ASSERT_EQ(mem.read16(0xFEDA), 0x3);
}

TEST(CPUTest, CALLCONDIMM16_NT) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store8(0x0, 0b11011100);
    mem.store16(0x1, 0x10FE);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x3);
    ASSERT_EQ(res.second, 3);
    ASSERT_EQ(cpu.SP(), 0xFEDC);
}

// OP(CALLIMM16     , 0b11001101, 0b11111111)
TEST(CPUTest, CALLIMM16) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store8(0x0, 0b11001101);
    mem.store16(0x1, 0x10FE);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x10FE);
    ASSERT_EQ(res.second, 6);
    ASSERT_EQ(cpu.SP(), 0xFEDA);
    ASSERT_EQ(mem.read16(0xFEDA), 0x3);
}

// OP(RSTTGT3       , 0b11000111, 0b11000111)
TEST(CPUTest, RSTTGT3) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store8(0x0, 0b11111111);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x38);
    ASSERT_EQ(res.second, 4);
    ASSERT_EQ(cpu.SP(), 0xFEDA);
    ASSERT_EQ(mem.read16(0xFEDA), 0x1);
}
//
// OP(POPR16STK     , 0b11000001, 0b11001111)
TEST(CPUTest, POPR16STK) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store16(0xFEDC, 0xABCD);
    mem.store8(0x0, 0b11000001);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 3);
    ASSERT_EQ(cpu.SP(), 0xFEDE);
    ASSERT_EQ(cpu.BC(), 0xABCD);
}

// OP(PUSHR16STK    , 0b11000101, 0b11001111)
TEST(CPUTest, PUSHR16STK) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    cpu.BC(0xABCD);
    mem.store8(0x0, 0b11000101);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 4);
    ASSERT_EQ(cpu.SP(), 0xFEDA);
    ASSERT_EQ(mem.read16(0xFEDA), 0xABCD);
}
//
// OP(LDHCA         , 0b11100010, 0b11111111)
TEST(CPUTest, LDHCA) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0xAF);
    cpu.C(0xF);
    mem.store8(0x0, 0b11100010);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(mem.read8(0xFF0F), 0xAF);
}

// OP(LDHIMM8A      , 0b11100000, 0b11111111)
TEST(CPUTest, LDHIMM8A) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0xAF);
    mem.store8(0x0, 0b11100000);
    mem.store8(0x1, 0xF);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 3);
    ASSERT_EQ(mem.read8(0xFF0F), 0xAF);
}

// OP(LDIMM16A      , 0b11101010, 0b11111111)
TEST(CPUTest, LDIMM16A) {
    GBCPU cpu;
    GBMEM mem;
    cpu.A(0xDC);
    mem.store8(0x0, 0b11101010);
    mem.store16(0x1, 0xFEDC);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x3);
    ASSERT_EQ(res.second, 4);
    ASSERT_EQ(mem.read8(0xFEDC), 0xDC);
}

// OP(LDHAC         , 0b11110010, 0b11111111)
TEST(CPUTest, LDHAC) {
    GBCPU cpu;
    GBMEM mem;
    cpu.C(0xF);
    mem.store8(0x0, 0b11110010);
    mem.store8(0xFF0F, 0xAF);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.A(), 0xAF);
}

// OP(LDHAIMM8      , 0b11110000, 0b11111111)
TEST(CPUTest, LDHAIMM8) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b11110000);
    mem.store8(0x1, 0xF);
    mem.store8(0xFF0F, 0xAF);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 3);
    ASSERT_EQ(cpu.A(), 0xAF);
}

// OP(LDAIMM16      , 0b11111010, 0b11111111)
TEST(CPUTest, LDAIMM16) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b11111010);
    mem.store16(0x1, 0xFEDC);
    mem.store8(0xFEDC, 0xDC);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x3);
    ASSERT_EQ(res.second, 4);
    ASSERT_EQ(cpu.A(), 0xDC);
}

//
// OP(ADDSPIMM8     , 0b11101000, 0b11111111)
TEST(CPUTest, ADDSPIMM8_PO) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store8(0x0, 0b11101000);
    mem.store8(0x1, 0b00000100); // +4
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 4);
    ASSERT_EQ(cpu.SP(), 0xFEE0);
}

TEST(CPUTest, ADDSPIMM8_NO) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store8(0x0, 0b11101000);
    mem.store8(0x1, 0b11111100); // -4
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 4);
    ASSERT_EQ(cpu.SP(), 0xFED8);
}

// OP(LDHLSPIMM8    , 0b11111000, 0b11111111)
TEST(CPUTest, LDHLSPIMM8_PO) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store8(0x0, 0b11111000);
    mem.store8(0x1, 0b00000100); // +4
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 3);
    ASSERT_EQ(cpu.SP(), 0xFEE0);
    ASSERT_EQ(cpu.HL(), cpu.SP());
}

TEST(CPUTest, LDHLSPIMM8_NO) {
    GBCPU cpu;
    GBMEM mem;
    cpu.SP(0xFEDC);
    mem.store8(0x0, 0b11111000);
    mem.store8(0x1, 0b11111100); // -4
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 3);
    ASSERT_EQ(cpu.SP(), 0xFED8);
    ASSERT_EQ(cpu.HL(), cpu.SP());
}

// OP(LDSPHL        , 0b11111001, 0b11111111)
TEST(CPUTest, LDSPHL) {
    GBCPU cpu;
    GBMEM mem;
    cpu.HL(0xABCD);
    mem.store8(0x0, 0b11111001);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.SP(), 0xABCD);
}

// OP(DI            , 0b11110011, 0b11111111)
TEST(CPUTest, DI) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b11110011);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    SKIP;
}
// OP(EI            , 0b11111011, 0b11111111)
TEST(CPUTest, EI) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b11111011);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x1);
    ASSERT_EQ(res.second, 1);
    SKIP;
}
//
// // ----------------------------
// //          BLOCK 4
// // ----------------------------
// OP(CB            , 0b11001011, 0b11111111)
// OP(RLCR8         , 0b00000000, 0b11111000)
TEST(CPUTest, RLCR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    cpu.B(0b01010101);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00000000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b10101010);
    ASSERT_FALSE(cpu.hasC());
}

TEST(CPUTest, RLCR8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    cpu.B(0b10101010);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00000000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b01010101);
    ASSERT_TRUE(cpu.hasC());
}

// OP(RRCR8         , 0b00001000, 0b11111000)
TEST(CPUTest, RRCR8) {
    GBCPU cpu;
    GBMEM mem;
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00001000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
}
// OP(RLR8          , 0b00010000, 0b11111000)
TEST(CPUTest, RLR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b01010101);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00010000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b10101010);
    ASSERT_FALSE(cpu.hasC());
}

TEST(CPUTest, RLR8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    cpu.B(0b10101010);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00010000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b01010101);
    ASSERT_TRUE(cpu.hasC());
}

// OP(RRR8          , 0b00011000, 0b11111000)
TEST(CPUTest, RRR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.setC();
    cpu.B(0b10101010);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00011000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b11010101);
    ASSERT_FALSE(cpu.hasC());
}

TEST(CPUTest, RRR8_C) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b01010101);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00011000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b00101010);
    ASSERT_TRUE(cpu.hasC());
}

// OP(SLAR8         , 0b00100000, 0b11111000)
TEST(CPUTest, SLAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b10000000);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00100000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0);
    ASSERT_TRUE(cpu.hasZ());
    ASSERT_TRUE(cpu.hasC());
}

// OP(SRAR8         , 0b00101000, 0b11111000)
TEST(CPUTest, SRAR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b10000001);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00101000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b11000000);
    ASSERT_TRUE(cpu.hasC());
}

// OP(SWAPR8        , 0b00110000, 0b11111000)
TEST(CPUTest, SWAPR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b10101100);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00110000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b11001010);
}
// OP(SRLR8         , 0b00111000, 0b11111000)
TEST(CPUTest, SRLR8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b10000001);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b00111000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b01000000);
    ASSERT_TRUE(cpu.hasC());
}
//
// OP(BITB3R8       , 0b01000000, 0b11000000)
TEST(CPUTest, BITB3R8_SET) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b00100000);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b01101000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_FALSE(cpu.hasZ());
}

TEST(CPUTest, BITB3R8_UNSET) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b11011111);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b01101000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_TRUE(cpu.hasZ());
}

// OP(RESB3R8       , 0b10000000, 0b11000000)
TEST(CPUTest, RESB3R8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b11111111);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b10101000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b11011111);
}

// OP(SETB3R8       , 0b11000000, 0b11000000)
TEST(CPUTest, SETB3R8) {
    GBCPU cpu;
    GBMEM mem;
    cpu.B(0b00000000);
    mem.store8(0x0, 0b11001011);
    mem.store8(0x1, 0b11101000);
    auto res = cpu.parseInstruction(mem, 0x0);
    ASSERT_EQ(res.first, 0x2);
    ASSERT_EQ(res.second, 2);
    ASSERT_EQ(cpu.B(), 0b00100000);
}
