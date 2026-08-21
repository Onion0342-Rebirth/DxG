#include "render/Camera.h"
#include <cmath>

namespace d25 {

void Camera::setViewport(int width, int height) {
    width_ = width > 0 ? width : 1;
    height_ = height > 0 ? height : 1;
    aspect_ = float(width_) / float(height_);
    rebuildProjection();
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
