#include "platform/SdlPlatform.h"
#include "core/PixelBuffer.h"
#include "input/Key.h"

#include <SDL.h>
#include <cstring>

namespace d25 {

namespace {
Key mapScancode(SDL_Scancode sc) {
    switch (sc) {
        case SDL_SCANCODE_W: return Key::W;
        case SDL_SCANCODE_A: return Key::A;
        case SDL_SCANCODE_S: return Key::S;
        case SDL_SCANCODE_D: return Key::D;
        case SDL_SCANCODE_X: return Key::X;
        case SDL_SCANCODE_Z: return Key::Z;
        case SDL_SCANCODE_Q: return Key::Q;
        case SDL_SCANCODE_UP:    return Key::Up;
        case SDL_SCANCODE_DOWN:  return Key::Down;
        case SDL_SCANCODE_LEFT:  return Key::Left;
        case SDL_SCANCODE_RIGHT: return Key::Right;
        case SDL_SCANCODE_SPACE:    return Key::Space;
        case SDL_SCANCODE_RETURN:   return Key::Enter;
        case SDL_SCANCODE_ESCAPE:   return Key::Escape;
        case SDL_SCANCODE_BACKSPACE:return Key::Backspace;
        default: return Key::None;
    }
}
} // namespace

SdlPlatform::SdlPlatform(int width, int height)
    : width_(width), height_(height) {}

SdlPlatform::~SdlPlatform() {
    shutdown();
}

bool SdlPlatform::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;

    window_ = SDL_CreateWindow(
        "DxG",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width_, height_, SDL_WINDOW_SHOWN);
    if (!window_) return false;

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) return false;

    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width_, height_);
    if (!texture_) return false;

    return true;
}

void SdlPlatform::shutdown() {
    if (texture_) { SDL_DestroyTexture(texture_); texture_ = nullptr; }
    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
    SDL_Quit();
}

bool SdlPlatform::pollEvent(InputEvent& out) {
    SDL_Event e;
    if (!SDL_PollEvent(&e)) return false;

    out = InputEvent{};
    if (e.type == SDL_QUIT) {
        out.type = InputEvent::Type::Quit;
    } else if (e.type == SDL_KEYDOWN) {
        out.type = InputEvent::Type::KeyDown;
        out.key = mapScancode(e.key.keysym.scancode);
    } else if (e.type == SDL_KEYUP) {
        out.type = InputEvent::Type::KeyUp;
        out.key = mapScancode(e.key.keysym.scancode);
    }
    return true;
}

void SdlPlatform::present(const PixelBuffer& frame) {
    if (!texture_ || frame.width() != width_ || frame.height() != height_) return;

    // PixelBuffer 为 RGBA，ARGB8888 在小端字节序下内存布局即 BGRA，
    // 这里逐像素转成 0xAARRGGBB。
    uint32_t* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(texture_, nullptr, reinterpret_cast<void**>(&pixels), &pitch) != 0) return;

    const int w = width_;
    const int h = height_;
    for (int y = 0; y < h; ++y) {
        uint32_t* row = reinterpret_cast<uint32_t*>(
            reinterpret_cast<uint8_t*>(pixels) + y * pitch);
        for (int x = 0; x < w; ++x) {
            const Color c = frame.get(x, y);
            row[x] = (uint32_t(c.a) << 24) | (uint32_t(c.r) << 16) |
                     (uint32_t(c.g) << 8)  |  uint32_t(c.b);
        }
    }
    SDL_UnlockTexture(texture_);
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

const char* SdlPlatform::getError() const {
    return SDL_GetError();
}

} // namespace d25
