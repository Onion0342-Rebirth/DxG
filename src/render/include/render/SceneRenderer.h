#pragma once
#include "core/Color.h"
#include "core/Vec.h"
#include "core/Mat.h"
#include "render/Camera.h"
#include "render/Rasterizer.h"
#include "world/terrain/TileMap.h"
#include <functional>
#include <vector>

namespace d25 {

class World;
class PixelBuffer;
class DepthBuffer;
class Sprite;
class SpriteSheet;

// 场景渲染所需的资产接口（端口）。
// render 层只认识这个接口；具体资源容器（res::ResourceManager）实现它，
// 从而保持 render -> world/core，不反向依赖 res。
class ISceneAssets {
public:
    virtual ~ISceneAssets() = default;
    virtual const Sprite* terrain(Terrain t) const = 0;
    virtual const Sprite* tree() const = 0;
    virtual const Sprite* rock() const = 0;
    virtual const SpriteSheet* playerSheet() const = 0;
};

// 场景渲染器：把 World + Camera 画进像素缓冲。只读玩法数据，不修改 World。
class SceneRenderer {
public:
    SceneRenderer(const World& world, Camera& camera, const ISceneAssets& assets);

    void render(PixelBuffer& color, DepthBuffer& depth);

    // 计算一个世界点在当前视图下的深度键（= 视空间 depth，越大越远）。
    static float viewDepthKey(const Mat4& view, const Vec3f& worldPos);

    // 按"相机右方向 + 相机上方向"构造屏幕对齐公告板四角（输出顺序：左上、右上、右下、
    // 左下，与 drawWorldQuad 的 UV 约定一致）。抽成静态纯函数以便无头单元测试：
    // rightDir 取 Camera::rightWorld()、upDir 取 Camera::upWorld()。这样公告板在相机
    // 空间严格沿 X/Y 轴，投影后水平边严格水平、竖直边严格竖直——角色即使在屏幕边缘
    // （地图边缘夹取、玩家偏离视线轴）也不会歪斜或被透视压扁，屏幕高宽比恒等于 w/h。
    static void billboardCorners(const Vec3f& rightDir, const Vec3f& upDir,
                                 const Vec3f& base, float w, float h, Vec3f out[4]);

private:
    struct DrawItem {
        float depth = 0.f;
        std::function<void()> draw;
    };

    void drawSky(PixelBuffer& color);
    void drawGround();
    void drawTiles();
    void drawActors();

    // 画一个由 4 个世界角点构成的四边形（UV 顺序：左上、右上、右下、左下）。
    void drawWorldQuad(const Vec3f corners[4], const float uv[8],
                       const Sprite* tex, Color tint);
    // 以 base 为地面中心点，画一个始终面向相机的公告板（w 为宽度，h 为高度）。
    void drawBillboard(const Vec3f& base, float w, float h,
                       const Sprite* tex, Color tint);
    // 画一个轴对齐盒子（min/max 世界坐标），顶面/侧面用不同亮度。
    void drawBox(const Vec3f& mn, const Vec3f& mx, Color side, Color top);

    static Color shade(Color c, float factor);
    static Color terrainFallback(Terrain t);

    const World& world_;
    Camera& cam_;
    const ISceneAssets& assets_;
    Rasterizer ras_;
    PixelBuffer* color_ = nullptr;
    DepthBuffer* depth_ = nullptr;
};

} // namespace d25
