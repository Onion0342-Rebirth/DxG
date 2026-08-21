#pragma once
#include "core/Color.h"
#include <vector>

namespace d25 {

// 帧缓冲（颜色）：软渲染输出目标。坐标原点在左上，y 向下。
class PixelBuffer {
public:
    PixelBuffer() = default;
    PixelBuffer(int w, int h) { resize(w, h); }

    void resize(int w, int h);

    int width() const { return width_; }
    int height() const { return height_; }
    bool empty() const { return width_ <= 0 || height_ <= 0; }

    Color get(int x, int y) const;
    void set(int x, int y, Color c);
    void fill(Color c);

    const Color* data() const { return data_.data(); }
    Color* data() { return data_.data(); }

private:
    int width_ = 0, height_ = 0;
    std::vector<Color> data_;
};

} // namespace d25
