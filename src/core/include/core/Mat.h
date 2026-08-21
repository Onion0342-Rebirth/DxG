#pragma once
#include "core/Vec.h"
#include <cmath>

namespace d25 {

// 列主序 4x4 矩阵：m[col][row]。用于视图/透视变换。
struct Mat4 {
    float m[4][4] = {};

    static Mat4 identity() {
        Mat4 r;
        for (int i = 0; i < 4; ++i) r.m[i][i] = 1.f;
        return r;
    }

    // 矩阵 * 列向量（世界坐标 -> 相机/裁剪空间）。
    static Vec4f mul(const Mat4& a, const Vec4f& v) {
        const float vv[4] = {v.x, v.y, v.z, v.w};
        float o[4] = {0.f, 0.f, 0.f, 0.f};
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                o[row] += a.m[col][row] * vv[col];
        return {o[0], o[1], o[2], o[3]};
    }

    // 右手系 lookAt：相机位于 eye，看向 target，上方向 up。相机空间朝向 -Z。
    static Mat4 lookAt(const Vec3f& eye, const Vec3f& target, const Vec3f& up) {
        const Vec3f f = (target - eye).normalized();
        const Vec3f s = cross(f, up).normalized();
        const Vec3f u = cross(s, f);
        Mat4 r;
        r.m[0][0] = s.x;  r.m[0][1] = s.y;  r.m[0][2] = s.z;  r.m[0][3] = -dot(s, eye);
        r.m[1][0] = u.x;  r.m[1][1] = u.y;  r.m[1][2] = u.z;  r.m[1][3] = -dot(u, eye);
        r.m[2][0] = -f.x; r.m[2][1] = -f.y; r.m[2][2] = -f.z; r.m[2][3] =  dot(f, eye);
        r.m[3][3] = 1.f;
        return r;
    }

    // 标准透视投影（深度映射到 -Z；本框架只用 clip.xy/w 做屏幕投影）。
    static Mat4 perspective(float fovDeg, float aspect, float nearPlane, float farPlane) {
        const float f = 1.f / std::tan(fovDeg * 0.5f * 3.14159265358979f / 180.f);
        Mat4 r;
        r.m[0][0] = f / aspect;
        r.m[1][1] = f;
        r.m[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        r.m[2][3] = -2.f * farPlane * nearPlane / (farPlane - nearPlane);
        r.m[3][2] = -1.f;
        return r;
    }
};

} // namespace d25
