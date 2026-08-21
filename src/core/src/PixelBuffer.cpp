#include "core/PixelBuffer.h"
#include <algorithm>

namespace d25 {

void PixelBuffer::resize(int w, int h) {
    width_ = w;
    height_ = h;
    data_.assign(size_t(w) * size_t(h), Color{0, 0, 0, 255});
}

Color PixelBuffer::get(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return {0, 0, 0, 255};
    return data_[size_t(y) * size_t(width_) + size_t(x)];
}

void PixelBuffer::set(int x, int y, Color c) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    data_[size_t(y) * size_t(width_) + size_t(x)] = c;
}

void PixelBuffer::fill(Color c) {
    std::fill(data_.begin(), data_.end(), c);
}

} // namespace d25
