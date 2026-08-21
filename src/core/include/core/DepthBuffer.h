#pragma once
#include <vector>

namespace d25 {

// 深度缓冲：逐像素存储视空间深度。默认全远；testAndSet 保留"更近"的深度。
class DepthBuffer {
public:
    DepthBuffer() = default;
    DepthBuffer(int w, int h) { resize(w, h); }

    void resize(int w, int h);

    int width() const { return width_; }
    int height() const { return height_; }

    void clear();

    // 若给定 depth 比当前更近，则写入并返回 true；否则返回 false。
    bool testAndSet(int x, int y, float depth);
    float get(int x, int y) const;

private:
    static constexpr float kFar = 1e30f;
    int width_ = 0, height_ = 0;
    std::vector<float> data_;
};

} // namespace d25
