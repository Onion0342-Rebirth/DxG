#include "core/DepthBuffer.h"
#include <algorithm>

namespace d25 {

void DepthBuffer::resize(int w, int h) {
    width_ = w;
    height_ = h;
    data_.assign(size_t(w) * size_t(h), kFar);
}

void DepthBuffer::clear() {
    std::fill(data_.begin(), data_.end(), kFar);
}

bool DepthBuffer::testAndSet(int x, int y, float depth) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return false;
    float& d = data_[size_t(y) * size_t(width_) + size_t(x)];
    if (depth < d) {
        d = depth;
        return true;
    }
    return false;
}

float DepthBuffer::get(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return kFar;
    return data_[size_t(y) * size_t(width_) + size_t(x)];
}

} // namespace d25
