// 分类：render
// 跟随相机纯函数测试（不依赖 SDL）：
//   1. 玩家在地图中央/内部 -> 注视点正好跟随玩家；
//   2. 玩家贴近/越过地图边缘 -> 注视点被夹取，相机地面可视范围不越出地图；
//   3. 视野高度变大 -> 地面可见边距单调增大（夹取得更早）；
//   4. 地图比视野还窄的极端情况 -> 该轴居中、不产生 NaN。
#include "framework/test_framework.h"
#include "render/FollowCam.h"
#include "render/Camera.h"
#include <cmath>

namespace {

using namespace d25;

constexpr float kMapW = 40.f;
constexpr float kMapH = 40.f;
constexpr float kAspect = 4.f / 3.f;

// 用真实相机投影：把地图边界点投影到屏幕，判断"相机不会看到地图外"。
// 这里直接验证夹取契约——夹取后，注视点应使地面可见梯形的四个角都落在地图范围内。
// 返回地面可见四角（世界坐标，y=0），通过 Camera 投影的逆思路：用 computeGroundFrustum 得到。
void groundCorners(const FollowCamConfig& cfg, float aspect,
                   const Vec2f& focus, Vec2f out[4]) {
    // 重新以"注视点为原点"的局部系求四角，再平移到世界。
    const float h = cfg.height, b = cfg.back;
    const Vec3f eye{0.f, h, b};
    const Vec3f fwd = (Vec3f{0.f, 0.f, 0.f} - eye).normalized();
    const Vec3f right = cross(fwd, {0.f, 1.f, 0.f}).normalized();
    const Vec3f camUp = cross(right, fwd).normalized();
    const float vfov = cfg.fovDeg * 3.14159265358979323846f / 180.f;
    const float tanV = std::tan(vfov * 0.5f);
    const float tanH = tanV * aspect;
    int i = 0;
    for (int sx = -1; sx <= 1; sx += 2) {
        for (int sy = -1; sy <= 1; sy += 2) {
            const Vec3f dir = (fwd + right * (float(sx) * tanH)
                                  + camUp * (float(sy) * tanV)).normalized();
            const float t = -eye.y / dir.y;
            out[i++] = Vec2f{eye.x + dir.x * t + focus.x,
                             eye.z + dir.z * t + focus.y};
        }
    }
}

bool nearly(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) < eps; }

} // namespace

void runFollowCamTests() {
    using namespace d25test;

    FollowCamConfig cfg; // 默认 height=9, back=15

    // 1. 视锥地面包围盒计算成功且为有限值。
    float minX = 0.f, maxX = 0.f, minZ = 0.f, maxZ = 0.f;
    beginCase("默认配置下视锥与地面相交并得到有效包围盒");
    bool ok = computeGroundFrustum(cfg, kAspect, minX, maxX, minZ, maxZ);
    CHECK(ok);
    CHECK(std::isfinite(minX) && std::isfinite(maxX) && std::isfinite(minZ) && std::isfinite(maxZ));
    CHECK(maxX > minX);
    CHECK(maxZ > minZ);
    std::printf("    [info] 地面可见包围盒 x∈[%.2f,%.2f] z∈[%.2f,%.2f]\n",
                minX, maxX, minZ, maxZ);

    // 2. 玩家在地图中央 -> 注视点跟随玩家。
    beginCase("玩家在地图中央时注视点=玩家位置");
    Vec2f center = computeFollowFocus(Vec2f{20.f, 20.f}, kMapW, kMapH, cfg, kAspect);
    CHECK(nearly(center.x, 20.f));
    CHECK(nearly(center.y, 20.f));

    // 3. 玩家在地图内部（远离边缘）-> 注视点跟随玩家。
    beginCase("玩家在地图内部时注视点跟随玩家");
    Vec2f inner = computeFollowFocus(Vec2f{22.f, 18.f}, kMapW, kMapH, cfg, kAspect);
    CHECK(nearly(inner.x, 22.f));
    CHECK(nearly(inner.y, 18.f));

    // 4. 玩家越过地图西南角(0,0) -> 注视点被夹取，不再等于玩家。
    beginCase("玩家越过西/南边时注视点被夹取（停在边缘内）");
    Vec2f sw = computeFollowFocus(Vec2f{-5.f, -5.f}, kMapW, kMapH, cfg, kAspect);
    CHECK(sw.x > -5.f);
    CHECK(sw.y > -5.f);

    // 5. 夹取后地面可见四角都在地图范围内（不出地图外）。
    beginCase("夹取后地面可见四角均在地图 [0,mapW]×[0,mapH] 内");
    Vec2f corners[4];
    const Vec2f fSW = computeFollowFocus(Vec2f{-5.f, -5.f}, kMapW, kMapH, cfg, kAspect);
    groundCorners(cfg, kAspect, fSW, corners);
    bool insideSW = true;
    for (int i = 0; i < 4; ++i) {
        if (corners[i].x < -0.01f || corners[i].x > kMapW + 0.01f ||
            corners[i].y < -0.01f || corners[i].y > kMapH + 0.01f) {
            insideSW = false;
        }
    }
    CHECK(insideSW);

    // 东北角同理。
    const Vec2f fNE = computeFollowFocus(Vec2f{999.f, 999.f}, kMapW, kMapH, cfg, kAspect);
    groundCorners(cfg, kAspect, fNE, corners);
    bool insideNE = true;
    for (int i = 0; i < 4; ++i) {
        if (corners[i].x < -0.01f || corners[i].x > kMapW + 0.01f ||
            corners[i].y < -0.01f || corners[i].y > kMapH + 0.01f) {
            insideNE = false;
        }
    }
    beginCase("玩家越过东/北边时夹取后可见四角仍在地图内");
    CHECK(insideNE);

    // 6. 玩家沿北边缘(z 小)行走时，注视点 z 被夹取、x 仍可跟随。
    beginCase("北边缘：z 被夹取而 x 仍跟随玩家横向移动");
    Vec2f a = computeFollowFocus(Vec2f{12.f, -3.f}, kMapW, kMapH, cfg, kAspect);
    Vec2f b = computeFollowFocus(Vec2f{28.f, -3.f}, kMapW, kMapH, cfg, kAspect);
    CHECK(nearly(a.x, 12.f));
    CHECK(nearly(b.x, 28.f));
    CHECK(nearly(a.y, b.y)); // z 都被夹到同一边界
    CHECK(a.y > -3.f);

    // 7. 视野高度变大 -> 地面可见半宽（X 边距）单调增大。
    beginCase("相机越高（视野越远）地面可见边距越大");
    FollowCamConfig low = cfg;  low.height = 6.f;  low.back = followBackForHeight(6.f);
    FollowCamConfig high = cfg; high.height = 16.f; high.back = followBackForHeight(16.f);
    float lMinX = 0.f, lMaxX = 0.f, lMinZ = 0.f, lMaxZ = 0.f;
    float hMinX = 0.f, hMaxX = 0.f, hMinZ = 0.f, hMaxZ = 0.f;
    computeGroundFrustum(low, kAspect, lMinX, lMaxX, lMinZ, lMaxZ);
    computeGroundFrustum(high, kAspect, hMinX, hMaxX, hMinZ, hMaxZ);
    const float lowHalfW = (lMaxX - lMinX) * 0.5f;
    const float highHalfW = (hMaxX - hMinX) * 0.5f;
    std::printf("    [info] 低视野半宽=%.2f  高视野半宽=%.2f\n", lowHalfW, highHalfW);
    CHECK(highHalfW > lowHalfW);

    // 8. 地图比视野还窄（极小地图）-> 居中且结果为有限值，不 NaN。
    beginCase("地图比视野窄时注视点居中且为有限值");
    Vec2f tiny = computeFollowFocus(Vec2f{0.3f, 0.3f}, 1.f, 1.f, cfg, kAspect);
    CHECK(std::isfinite(tiny.x) && std::isfinite(tiny.y));
    CHECK(nearly(tiny.x, 0.5f));
    CHECK(nearly(tiny.y, 0.5f));

    // 9. followEyeTarget 生成的机位：俯视、在注视点后方 +Z、高度取配置。
    beginCase("followEyeTarget 机位在注视点后上方、俯角固定");
    Vec3f eye, target;
    followEyeTarget(Vec2f{20.f, 20.f}, cfg, eye, target);
    CHECK(nearly(eye.x, 20.f));
    CHECK(nearly(eye.y, cfg.height));
    CHECK(nearly(eye.z, 20.f + cfg.back));
    CHECK(nearly(target.x, 20.f) && nearly(target.y, 0.f) && nearly(target.z, 20.f));
}
