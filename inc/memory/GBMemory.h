#pragma once

#include <array>
#include <cstdint>

class GBMEM {
    public:
        GBMEM() = default;
        ~GBMEM() = default;
        enum REG {
            P1      = 0xFF00, JOYP = 0xFF00,
            SB      = 0xFF01,
            SC      = 0xFF02,
            DIV     = 0xFF04,
            TIMA    = 0xFF05,
            TMA     = 0xFF06,
            TAC     = 0xFF07,
            IF      = 0xFF10,
            NR10    = 0xFF11,
            NR11    = 0xFF12,
            NR12    = 0xFF13,
            NR13    = 0xFF14,
            NR14    = 0xFF16,
            NR21    = 0xFF17,
            NR22    = 0xFF18,
            NR23    = 0xFF19,
            NR24    = 0xFF1A,
            NR30    = 0xFF1B,
            NR31    = 0xFF1C,
            NR32    = 0xFF1D,
            NR33    = 0xFF1E,
            NR34    = 0xFF20,
            NR41    = 0xFF21,
            NR42    = 0xFF22,
            NR43    = 0xFF23,
            NR44    = 0xFF24,
            NR50    = 0xFF25,
            NR51    = 0xFF26,
            WAVE_START = 0xFF30, WAVE_END = 0xFF3F,
            LDCC    = 0xFF40,
            STAT    = 0xFF41,
            SCY     = 0xFF42,
            SCX     = 0xFF43,
            LY      = 0xFF44,
            LYC     = 0xFF45,
            DMA     = 0xFF46,
            BGP     = 0xFF47,
            OBP0    = 0xFF48,
            OBP1    = 0xFF49,
            WY      = 0xFF4A,
            WX      = 0xFF4B,
            KEY0    = 0xFF4C, SYS = 0xFF4C,
            KEY1    = 0xFF4D, SPD = 0xFF4D,
            VBK     = 0xFF4F,
            BANK    = 0xFF50,
            HDMA1   = 0xFF51,
            HDMA2   = 0xFF52,
            HDMA3   = 0xFF53,
            HDMA4   = 0xFF54,
            HDMA5   = 0xFF55,
            RP      = 0xFF56,
            BCPS    = 0xFF68, BGPI = 0xFF68,
            BCPD    = 0xFF69, BGPD = 0xFF69,
            OCPS    = 0xFF6A, OBPI = 0xFF6A,
            OCPD    = 0xFF6B, OBPD = 0xFF6B,
            OPRI    = 0xFF6C,
            SVBK    = 0xFF70, WBK = 0xFF70,
            PCM12   = 0xFF76,
            PCM34   = 0xFF77,
            IE      = 0xFFFF
        };
        uint8_t read8(uint16_t address) const;
        void store8(uint16_t address, uint8_t data);
        uint16_t read16(uint16_t address) const;
        void store16(uint16_t address, uint16_t data);
        bool isOAM(uint16_t address) const;
        bool isVRAM(uint16_t address) const;
        bool isPalletes(uint16_t address) const;
        void lockOAM();
        void lockVRAM();
        void lockPallettes();
        void unlockOAM();
        void unlockVRAM();
        void unlockPallettes();
    private:
        bool _OAM_locked = false;
        bool _VRAM_locked = false;
        bool _PAL_locked = false;
        std::array<uint8_t, 0x10000> _MEM;
};
