#pragma once
#include "core/Color.h"
#include "render/Camera.h"

namespace d25 {

class PixelBuffer;
class DepthBuffer;
class Sprite;

// 软件三角形光栅器：
//   - 边函数扫描，CCW 为正面
//   - 重心坐标做透视校正（1/depth 线性插值，再除回 UV）
//   - z-buffer（更小 depth=更近，写入）
//   - 距离雾：颜色向雾色按 depth 线性插值
//   - 纹素 alpha==0 视为透明（跳过写色，但仍写 depth）
class Rasterizer {
public:
    void begin(PixelBuffer& color, DepthBuffer& depth);

    void setFog(Color c, float start, float end);
    void disableFog() { fogEnabled_ = false; }

    // 画三角形 a/b/c；tex 可空（用 tint 纯色）；顶点顺序 CCW 为正面。
    void triangle(const ScreenVertex& a, const ScreenVertex& b, const ScreenVertex& c,
                  const Sprite* tex, Color tint);

private:
    PixelBuffer* color_ = nullptr;
    DepthBuffer* depth_ = nullptr;

    bool fogEnabled_ = false;
    Color fogColor_{228, 240, 250, 255};
    float fogStart_ = 18.f;
    float fogEnd_ = 90.f;
};

} // namespace d25
