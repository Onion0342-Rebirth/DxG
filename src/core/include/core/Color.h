#pragma once
#include <cstdint>

namespace d25 {

// 32 位 RGBA 像素。
struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;

    static Color rgb(uint8_t r, uint8_t g, uint8_t b) { return {r, g, b, 255}; }
    static Color rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return {r, g, b, a}; }

    // 线性插值（t in [0,1]）。
    static Color lerp(const Color& a, const Color& b, float t) {
        return {uint8_t(a.r + (int(b.r) - int(a.r)) * t),
                uint8_t(a.g + (int(b.g) - int(a.g)) * t),
                uint8_t(a.b + (int(b.b) - int(a.b)) * t),
                uint8_t(a.a + (int(b.a) - int(a.a)) * t)};
    }
};

} // namespace d25
