#include <gtest/gtest.h>
#include <memory/GBMemory.h>

TEST(MEMTest, DefaultTest) {
    GBMEM mem;
    ASSERT_EQ(mem.read8(0xFF), 0);
    ASSERT_EQ(mem.read16(0xF), 0);
}

TEST(MEMTest, ReadAndWrite8) {
    GBMEM mem;
    mem.store8(0xF, 0x1);
    ASSERT_EQ(mem.read8(0xF), 0x1);
}

TEST(MEMTest, ReadAndWrite16) {
    GBMEM mem;
    mem.store16(0xF, 0x0F01);
    ASSERT_EQ(mem.read8(0xF), 0x01);
    ASSERT_EQ(mem.read8(0x10), 0x0F);
    ASSERT_EQ(mem.read16(0xF), 0x0F01);
}
