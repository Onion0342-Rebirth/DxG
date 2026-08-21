#include "render/Sprite.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace d25 {

void Sprite::resize(int w, int h) {
    width_ = w > 0 ? w : 0;
    height_ = h > 0 ? h : 0;
    pixels_.assign(size_t(width_) * size_t(height_), Color{0, 0, 0, 0});
}

void Sprite::clear(Color c) {
    std::fill(pixels_.begin(), pixels_.end(), c);
}

Color Sprite::pixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return {0, 0, 0, 0};
    return pixels_[size_t(y) * size_t(width_) + size_t(x)];
}

void Sprite::setPixel(int x, int y, Color c) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    pixels_[size_t(y) * size_t(width_) + size_t(x)] = c;
}

Color Sprite::sample(float u, float v) const {
    if (empty()) return {0, 0, 0, 0};
    // 最近邻：像素中心在 (i+0.5)/w
    int sx = int(std::floor(u * width_ - 0.5f + 0.5f));
    int sy = int(std::floor(v * height_ - 0.5f + 0.5f));
    sx = std::clamp(sx, 0, width_ - 1);
    sy = std::clamp(sy, 0, height_ - 1);
    return pixel(sx, sy);
}

void Sprite::fromAscii(const char* const* rows, int rowCount, const Color* palette, int paletteCount) {
    if (rowCount <= 0 || !rows) return;
    int w = 0;
    for (int y = 0; y < rowCount; ++y) {
        const int len = rows[y] ? int(std::strlen(rows[y])) : 0;
        w = std::max(w, len);
    }
    resize(w, rowCount);
    for (int y = 0; y < rowCount; ++y) {
        if (!rows[y]) continue;
        const int len = int(std::strlen(rows[y]));
        for (int x = 0; x < len; ++x) {
            const char ch = rows[y][x];
            if (ch == '.' || ch == ' ') continue; // 透明
            const int idx = (ch >= 'A' && ch <= 'Z') ? (ch - 'A') : -1;
            if (idx >= 0 && idx < paletteCount) {
                setPixel(x, y, palette[idx]);
            }
        }
    }
}

} // namespace d25
