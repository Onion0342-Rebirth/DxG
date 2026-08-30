#include "render/Camera.h"
#include <cmath>

namespace d25 {

void Camera::setViewport(int width, int height) {
    width_ = width > 0 ? width : 1;
    height_ = height > 0 ? height : 1;
    aspect_ = float(width_) / float(height_);
    rebuildProjection();
}

Vec3f Camera::rightWorld() const {
    // 视图矩阵把世界坐标旋到相机空间，其数学行向量即世界系基向量 s/u/-f；
    // 本矩阵按列主序 m[col][row] 存储，故右基向量 s（数学行 0）跨列 0..2 读取。
    return Vec3f{view_.m[0][0], view_.m[1][0], view_.m[2][0]};
}

Vec3f Camera::upWorld() const {
    // 上基向量 u（数学行 1）：列主序下跨列读取 m[0..2][1]。
    return Vec3f{view_.m[0][1], view_.m[1][1], view_.m[2][1]};
}

void Camera::configure(const Vec3f& eye, const Vec3f& target, float fovDeg) {
    eye_ = eye;
    fov_ = fovDeg;
    view_ = Mat4::lookAt(eye, target, {0.f, 1.f, 0.f});
    rebuildProjection();
}

void Camera::rebuildProjection() {
    proj_ = Mat4::perspective(fov_, aspect_, near_, far_);
}

bool Camera::projectWorld(const Vec3f& p, ScreenVertex& out) const {
    const Vec4f cam = Mat4::mul(view_, {p.x, p.y, p.z, 1.f});
    return projectCamUV({cam.x, cam.y, cam.z}, 0.f, 0.f, out);
}

bool Camera::projectCamUV(const Vec3f& camPos, float u, float v, ScreenVertex& out) const {
    const float depth = -camPos.z;
    if (depth < near_) return false;

    const Vec4f clip = Mat4::mul(proj_, {camPos.x, camPos.y, camPos.z, 1.f});
    if (clip.w == 0.f) return false;
    const float ndcX = clip.x / clip.w;
    const float ndcY = clip.y / clip.w;

    out.x = (ndcX * 0.5f + 0.5f) * float(width_);
    out.y = (0.5f - ndcY * 0.5f) * float(height_);
    out.invDepth = 1.f / depth;
    out.uOverZ = u * out.invDepth;
    out.vOverZ = v * out.invDepth;
    return true;
}

} // namespace d25
