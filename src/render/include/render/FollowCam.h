#pragma once
// 跟随相机纯函数：根据玩家在地面（XZ 平面）的位置，计算夹取到地图范围内的相机注视点/机位。
//
// 设计为与渲染/SDL 解耦的纯函数（仅依赖 core/Vec.h），便于无头单元测试：
//   - 相机俯视角度由配置固定（height = 离地高度，back = 注视点后方 +Z 水平距离）；
//   - 视野（height）越大，视锥在地面的覆盖范围越大；
//   - 通过"视锥四条角射线与 y=0 地面的交点"求出地面可见梯形，取其包围盒作为边距，
//     再把注视点 clamp 到 [可见近边, 地图尺寸-可见远边]，从而保证屏幕边缘不越出地图外。
//
// 坐标系约定与世界一致：玩法逻辑用 XZ 平面（Vec2f 的 y 分量即世界 z），Y 轴向上。
#include "core/Vec.h"
#include <cmath>
#include <algorithm>

namespace d25 {

// 跟随相机可调参数。玩法层持有一份，暂停菜单共享同一份以实时调整视野。
//
// 俯角取约 45°（back ≈ height，与角色素材截图所采用的取景角度一致）。
// 原因：斜视角下视锥顶边接近地平线，地面可见纵深会非常大；若用原来更平缓的
// 约 31° 俯角，地面可见范围（纵深 40+ 米）会超过 40×40 地图，相机无法靠夹取把
// 画面限制在地图内。约 45° 时可见地面小于地图，跟随 + 边缘夹取才有意义。
struct FollowCamConfig {
    float height = 9.0f;   // 相机离地高度（米）；越大视野越远（原固定机位为 12，现默认 9 更近）
    float back   = 9.0f;   // 相机在注视点后方（+Z）的水平距离；与 height 联动保持约 45° 俯角
    float fovDeg = 42.0f;  // 垂直视野（度），沿用相机默认

    // 视野滑块可调的高度范围（暂停菜单 VIEW 滑块）。
    float viewHeightMin = 6.0f;
    float viewHeightMax = 16.0f;
};

// 计算视锥四条角射线与地面（y=0）的交点，得到地面可见梯形的包围盒。
// 以"注视点位于原点、相机看向 -Z"的局部系计算，故包围盒也是相对注视点的偏移。
// 成功返回 true；当相机看向水平以上（近射线打不到地面）时返回 false（调用方应居中）。
//   outMinX/outMaxX：可见区域相对注视点的 X 方向最小/最大偏移；
//   outMinZ/outMaxZ：可见区域相对注视点的 Z 方向最小/最大偏移（负=近处，正=远处）。
inline bool computeGroundFrustum(const FollowCamConfig& cfg, float aspect,
                                 float& outMinX, float& outMaxX,
                                 float& outMinZ, float& outMaxZ) {
    const float h = cfg.height;
    const float b = cfg.back;
    if (h <= 0.f || b <= 0.f || aspect <= 0.f) return false;

    // 视线方向（从 eye 指向 target）：水平朝 -Z、竖直朝下。
    const Vec3f eye{0.f, h, b};
    const Vec3f fwd = (Vec3f{0.f, 0.f, 0.f} - eye).normalized(); // (0,-h,-b)/L
    const Vec3f up{0.f, 1.f, 0.f};
    const Vec3f right = cross(fwd, up).normalized();
    // 相机上向量（与视线、右向量正交）。
    const Vec3f camUp = cross(right, fwd).normalized();

    const float vfov = cfg.fovDeg * 3.14159265358979323846f / 180.f;
    const float tanV = std::tan(vfov * 0.5f);
    const float tanH = tanV * aspect;

    float minX = 0.f, maxX = 0.f, minZ = 0.f, maxZ = 0.f;
    bool any = false;
    // 视锥四个角：右/左 × 上/下。
    for (int sx = -1; sx <= 1; sx += 2) {
        for (int sy = -1; sy <= 1; sy += 2) {
            const Vec3f dir = (fwd + right * (float(sx) * tanH)
                                  + camUp * (float(sy) * tanV)).normalized();
            if (dir.y >= -1e-6f) continue; // 射线水平/朝上，与 y=0 无交点
            // p = eye + t*dir，令 p.y = 0 -> t = -eye.y / dir.y
            const float t = -eye.y / dir.y;
            const float gx = eye.x + dir.x * t;
            const float gz = eye.z + dir.z * t;
            if (!any) { minX = maxX = gx; minZ = maxZ = gz; any = true; }
            else {
                minX = std::min(minX, gx); maxX = std::max(maxX, gx);
                minZ = std::min(minZ, gz); maxZ = std::max(maxZ, gz);
            }
        }
    }
    if (!any || maxX <= minX || maxZ <= minZ) return false;
    outMinX = minX; outMaxX = maxX; outMinZ = minZ; outMaxZ = maxZ;
    return true;
}

// 依据玩家地面 XZ 坐标、地图世界尺寸与配置，计算夹取后的相机注视点（XZ，y=0）。
// playerXZ.y 表示世界 z；mapW/mapH 为地图世界尺寸（瓦片数 × tileSize）。
inline Vec2f computeFollowFocus(const Vec2f& playerXZ, float mapW, float mapH,
                                const FollowCamConfig& cfg, float aspect) {
    Vec2f focus{mapW * 0.5f, mapH * 0.5f}; // 默认居中（视野比地图还大时）

    float minX, maxX, minZ, maxZ;
    if (computeGroundFrustum(cfg, aspect, minX, maxX, minZ, maxZ)) {
        // 可见区域相对注视点的偏移：x∈[minX,maxX]、z∈[minZ,maxZ]。
        // 要求可见区域不越出地图 [0,mapW]×[0,mapH]，则注视点范围：
        //   x ∈ [-minX, mapW-maxX]，z ∈ [-minZ, mapH-maxZ]。
        const float loX = -minX, hiX = mapW - maxX;
        const float loZ = -minZ, hiZ = mapH - maxZ;
        if (loX < hiX) focus.x = std::max(loX, std::min(hiX, playerXZ.x));
        if (loZ < hiZ) focus.y = std::max(loZ, std::min(hiZ, playerXZ.y));
    }
    return focus;
}

// 由注视点（XZ）与配置生成相机 eye/target：俯角固定，视野调整只改高度/后拉距离。
inline void followEyeTarget(const Vec2f& focusXZ, const FollowCamConfig& cfg,
                            Vec3f& eye, Vec3f& target) {
    target = Vec3f{focusXZ.x, 0.f, focusXZ.y};
    eye    = Vec3f{focusXZ.x, cfg.height, focusXZ.y + cfg.back};
}

// 视野高度按固定俯角联动后拉距离（back = height，约 45° 俯角），
// 使滑块只改变"远近"而不改变俯视角度。
inline float followBackForHeight(float height) {
    return height;
}

} // namespace d25
