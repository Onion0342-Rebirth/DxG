#pragma once
#include "core/Color.h"
#include <cstdint>
#include <string>
#include <vector>

namespace d25 {

// 像素精灵：宽 x 高的 RGBA 像素表。
// 约定：sample 的 v=0 是顶部、v=1 是底部；透明像素 alpha=0。
class Sprite {
public:
    Sprite() = default;
    Sprite(int w, int h) { resize(w, h); }

    void resize(int w, int h);

    int width() const { return width_; }
    int height() const { return height_; }
    bool empty() const { return width_ <= 0 || height_ <= 0; }

    void clear(Color c = {0, 0, 0, 0});

    Color pixel(int x, int y) const;
    void setPixel(int x, int y, Color c);

    // 最近邻采样（u,v ∈ [0,1]；越界 clamp）。
    Color sample(float u, float v) const;

    // 字符画构造：'.'（或空格）=透明；'A'-'Z' 取 palette 索引 (ch-'A')。
    void fromAscii(const char* const* rows, int rowCount, const Color* palette, int paletteCount);

private:
    int width_ = 0, height_ = 0;
    std::vector<Color> pixels_;
};

} // namespace d25
