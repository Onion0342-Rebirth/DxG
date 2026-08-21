#pragma once
#include "core/Vec.h"
#include "core/Mat.h"

namespace d25 {

// 屏幕空间顶点：投影后的 2D 坐标 + 透视校正插值所需的量。
//   invDepth = 1 / view-space depth（屏幕空间线性）
//   uOverZ/vOverZ = u / depth、v / depth，在屏幕空间线性插值后再除以 invDepth 还原 UV。
struct ScreenVertex {
    float x = 0.f, y = 0.f;
    float invDepth = 1.f;
    float uOverZ = 0.f, vOverZ = 0.f;
};

// 固定透视相机：一个场景 configure 一次，不跟随角色。
// 右手系、看向 -Z；view-space depth = -cam.z（越大越远）。
class Camera {
public:
    void setViewport(int width, int height);
    int width() const { return width_; }
    int height() const { return height_; }

    // eye: 相机世界坐标；target: 注视点；fovDeg: 垂直视野。
    void configure(const Vec3f& eye, const Vec3f& target, float fovDeg);

    const Mat4& view() const { return view_; }
    const Mat4& proj() const { return proj_; }
    const Vec3f& eye() const { return eye_; }
    float fov() const { return fov_; }
    float nearPlane() const { return near_; }
    float farPlane() const { return far_; }

    // 世界点投影到屏幕（不携带 UV，用于排序/可见性判断）。在近平面后返回 false。
    bool projectWorld(const Vec3f& p, ScreenVertex& out) const;

    // 相机空间点 + 顶点 UV 投影到屏幕（携带透视校正量）。
    bool projectCamUV(const Vec3f& camPos, float u, float v, ScreenVertex& out) const;

private:
    void rebuildProjection();

    int width_ = 800, height_ = 600;
    float fov_ = 42.f;
    float near_ = 0.3f, far_ = 300.f;
    float aspect_ = 1.f;
    Vec3f eye_{0.f, 0.f, 0.f};
    Mat4 view_;
    Mat4 proj_;
};

} // namespace d25
