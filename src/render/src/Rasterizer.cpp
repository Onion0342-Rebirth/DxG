#include "render/Rasterizer.h"
#include "core/PixelBuffer.h"
#include "core/DepthBuffer.h"
#include "render/Sprite.h"
#include <algorithm>
#include <cmath>

namespace d25 {

void Rasterizer::begin(PixelBuffer& color, DepthBuffer& depth) {
    color_ = &color;
    depth_ = &depth;
    depth_->clear();
}

void Rasterizer::setFog(Color c, float start, float end) {
    fogColor_ = c;
    fogStart_ = start;
    fogEnd_ = end;
    fogEnabled_ = end > start;
}

namespace {
// 有向边函数：返回 (p-a)×(b-a) 的 2D 叉积。
inline float edgeFn(float ax, float ay, float bx, float by, float px, float py) {
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}
} // namespace

void Rasterizer::triangle(const ScreenVertex& a, const ScreenVertex& b,
                          const ScreenVertex& c, const Sprite* tex, Color tint) {
    if (!color_ || !depth_) return;

    // 屏幕包围盒（clamp 到视口）。
    const int minX = std::clamp(int(std::floor(std::min({a.x, b.x, c.x}))), 0, color_->width() - 1);
    const int maxX = std::clamp(int(std::ceil (std::max({a.x, b.x, c.x}))), 0, color_->width() - 1);
    const int minY = std::clamp(int(std::floor(std::min({a.y, b.y, c.y}))), 0, color_->height() - 1);
    const int maxY = std::clamp(int(std::ceil (std::max({a.y, b.y, c.y}))), 0, color_->height() - 1);
    if (minX > maxX || minY > maxY) return;

    const float area = edgeFn(a.x, a.y, b.x, b.y, c.x, c.y);
    if (std::abs(area) < 1e-6f) return;
    const float invArea = 1.0f / area;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            // 像素中心。
            const float px = float(x) + 0.5f;
            const float py = float(y) + 0.5f;

            const float w0 = edgeFn(b.x, b.y, c.x, c.y, px, py) * invArea;
            const float w1 = edgeFn(c.x, c.y, a.x, a.y, px, py) * invArea;
            const float w2 = edgeFn(a.x, a.y, b.x, b.y, px, py) * invArea;

            // 屏幕空间：背面剔除由 area 符号 + 重心同号决定（CCW 正面）。
            if (w0 < 0.f || w1 < 0.f || w2 < 0.f) continue;

            // 透视校正：invDepth 与 u/z、v/z 在屏幕空间线性。
            const float invDepth = w0 * a.invDepth + w1 * b.invDepth + w2 * c.invDepth;
            if (invDepth <= 0.f) continue;
            const float depth = 1.0f / invDepth;

            if (!depth_->testAndSet(x, y, depth)) continue;

            // 还原 UV：u = (u/z) / (1/z)。
            const float uOverZ = w0 * a.uOverZ + w1 * b.uOverZ + w2 * c.uOverZ;
            const float vOverZ = w0 * a.vOverZ + w1 * b.vOverZ + w2 * c.vOverZ;
            const float u = uOverZ * depth;
            const float v = vOverZ * depth;

            Color out = tint;
            if (tex && !tex->empty()) {
                const Color s = tex->sample(u, v);
                if (s.a == 0) {
                    // 透明像素：不写颜色，但已写 depth（后面的像素不会再穿过它）。
                    continue;
                }
                // 简单调制（tint 为白时等价贴图原色）。
                out = Color{
                    uint8_t((s.r * tint.r) / 255),
                    uint8_t((s.g * tint.g) / 255),
                    uint8_t((s.b * tint.b) / 255),
                    uint8_t((s.a * tint.a) / 255),
                };
            }

            if (fogEnabled_) {
                float t = (depth - fogStart_) / (fogEnd_ - fogStart_);
                t = std::clamp(t, 0.f, 1.f);
                out = Color::lerp(out, fogColor_, t);
            }

            color_->set(x, y, out);
        }
    }
}

} // namespace d25
