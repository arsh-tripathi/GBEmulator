#pragma once
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <cstdint>

class GBScreen {
    // TILES: Groups of 8x8 pixels
    // PIXEL: 2 bit per, 0 to 3
    //        1-3: palette
    //        0: transparent
    // PALLETTES: 4 colors
    // LAYERS: Background, Window, Objects
    // Background: Tilemap i.e. large gried of tiles
    // Window: Second bg layer on top of background
    //         No transparency, only top-left pixel
    //         controlled
    // Objects: Objects made of 1-2 stacked tiles
    //          Can be displayed anywhere on the screen
    // VRAM Tile data: Stored in memory area 0x8000 - 0x97FF
    //      Each tile 16 bytes, 384 tiles
    //      Three blocks of 128 tiles each
    //      Block 0: 0x8000 - 0x87FF
    //      Block 1: 0x8800 - 0x8FFF
    //      Block 2: 0x9000 - 0x97FF
    // Two addressing methods:
    // 0x8000 base: unsigned offsets, always used for objects
    // 0x9000 base: signed offsets, LCDC bit 4 = 0 enables
    // Tile data format: 16 bytes, pairs of two for each row
    //      first byte least significant digits per pixel
    //      second byte most significant digits per pixel
    //      Color index per pixel 0-3
    //      Color map depends on pallete defined in 
    //      BGP, OBP0, OBP1
    //      For objects 0 = transparent
    // VRAM Tile Maps: two 32x32 tile maps in 
    //                 0x9800 - 0x9BFF and 0x9C00 - 0x9FFF
    //                 Each map contains 1 byte index of tiles to display
    // SCY and SCX registers used to scroll background, specify origin 
    // of the visible 160x144 area, visible area wraps around map
    // In non-cgb can be disabled using LCDC bit 0
    // Window: Non scrollable, modify position using WX and WY
    // Top left corner coordinates (WX - 7, WY)
    // Window visibility can be toggled by LCDC bit 5
    // Window internal line counter functionally similar to LY, 
    // increments algonside it. Only incremented when window visible
    // Object: Same format as BG Tiles, taken from tile blocks 0 and 1
    // Object attributes reside in OAM: 0xFE00 - 0xFE9F
    // 40 objects, 10 max per scan line, each object 4 bytes
    // Byte 0: Y Position = Vertical pos on the screen + 16
    //         Y = 0 Hides all objects even 8x16
    // Byte 1: X Position = Horizontal pos on screen + 8
    // Byte 2: Tile Index: 
    //          In 8x8 mode (LCDC bit 2 = 0) specifies only tile index
    //          Unsigned value selects from 0x8000 - 0x8FFF
    //          In 8x16 mode (LCDC bit 2 = 1) specifies index of top tile
    //          Least significant bit ignored, top tile is NN & 0xFE
    //          bottom tile is NN | 0x01
    // Byte 3: Attributes/Flags
    //      Bit 7: Priority: 0 = No 1 = BG and Window(1-3) drawn over this OBJ
    //      Bit 6: Y Flip: 0 = Normal 1 = Entire OBJ is vertically mirrored
    //      Bit 5: X Flip: 0 = Normal 1 = Entire OBJ is horizontally mirrored
    //      Bit 4: DMG pallete: 0 = OBP0 1 = OBP1
    //      Bit 3-0: CGB Stuff
    // Writing to OAM: Write to buffer in normal RAM (usually WRAM) and copy
    // to OAM using DMA transfer func
    static const uint8_t SCR_WIDTH      = 160;
    static const uint8_t SCR_HEIGHT     = 144;
    static const uint32_t TRANSPARENT   = 0x00000000;
    static const uint32_t WHITE         = 0xFFFFFFFF;
    static const uint32_t LIGHT_GRAY    = 0x8C8C8CFF;
    static const uint32_t DARK_GRAY     = 0x323232FF;
    static const uint32_t BLACK         = 0x000000FF;

    uint16_t WIDTH = SCR_WIDTH;
    uint16_t HEIGHT = SCR_HEIGHT;
    uint8_t SCALE = 16; // Actual scale * 16, avoid floating point issues

    uint32_t framebuffer[SCR_HEIGHT][SCR_WIDTH];

    SDL_Texture *texture = nullptr;
    SDL_Renderer* renderer = nullptr;
    GBScreen(SDL_Renderer* renderer);
    SDL_FRect getDestRect();

    void resize(uint16_t new_width, uint16_t new_height);
    void draw();
    void step(uint8_t dots);
  private:
    uint8_t line = 0;
    uint8_t currDots = 0;
};
