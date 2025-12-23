#include <GBScreen.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <algorithm>
#include <cstdint>

GBScreen::GBScreen(SDL_Renderer* renderer): renderer{renderer} {
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            WIDTH,
            HEIGHT
    );
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
}

SDL_FRect GBScreen::getDestRect() {
    return SDL_FRect(
        static_cast<float>(WIDTH - 10 * SCALE) / 2,
        static_cast<float>(HEIGHT - 9 * SCALE) / 2,
        10 * SCALE,
        9 * SCALE
    );
}

void GBScreen::resize(uint16_t new_width, uint16_t new_height) {
    WIDTH = std::max(new_width, static_cast<uint16_t>(SCR_WIDTH));
    HEIGHT = std::max(new_height, static_cast<uint16_t>(SCR_HEIGHT));
    SCALE = std::min(WIDTH / 10, HEIGHT / 9);
}

void GBScreen::draw() {
    SDL_UpdateTexture(texture, nullptr, framebuffer, 160 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_FRect dstrect = getDestRect();
    SDL_RenderTexture(renderer, texture, nullptr, &dstrect);
    // SDL_RenderPresent(renderer); Call this per frame
}

void GBScreen::step(uint8_t dots) {
}
