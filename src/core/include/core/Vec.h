#pragma once
#include <cmath>

namespace d25 {

// ---------- 2D 向量（玩法逻辑层：XZ 平面坐标用） ----------
struct Vec2f {
    float x = 0.f, y = 0.f;

    Vec2f() = default;
    Vec2f(float x_, float y_) : x(x_), y(y_) {}

    Vec2f operator+(const Vec2f& o) const { return {x + o.x, y + o.y}; }
    Vec2f operator-(const Vec2f& o) const { return {x - o.x, y - o.y}; }
    Vec2f operator*(float s) const { return {x * s, y * s}; }
    Vec2f operator/(float s) const { return {x / s, y / s}; }
    Vec2f& operator+=(const Vec2f& o) { x += o.x; y += o.y; return *this; }
    Vec2f& operator-=(const Vec2f& o) { x -= o.x; y -= o.y; return *this; }

    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSq() const { return x * x + y * y; }
    Vec2f normalized() const {
        const float l = length();
        return l > 1e-6f ? Vec2f{x / l, y / l} : Vec2f{0.f, 0.f};
    }
};

// ---------- 3D 向量（渲染/世界高度用） ----------
struct Vec3f {
    float x = 0.f, y = 0.f, z = 0.f;

    Vec3f() = default;
    Vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3f operator+(const Vec3f& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3f operator-(const Vec3f& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3f operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3f& operator+=(const Vec3f& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3f& operator-=(const Vec3f& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }

    float length() const { return std::sqrt(x * x + y * y + z * z); }
    float lengthSq() const { return x * x + y * y + z * z; }
    Vec3f normalized() const {
        const float l = length();
        return l > 1e-6f ? Vec3f{x / l, y / l, z / l} : Vec3f{0.f, 0.f, 0.f};
    }
};

// ---------- 4D 齐次向量（矩阵运算用） ----------
struct Vec4f {
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;

    Vec4f() = default;
    Vec4f(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

inline float dot(const Vec2f& a, const Vec2f& b) { return a.x * b.x + a.y * b.y; }
inline float dot(const Vec3f& a, const Vec3f& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3f cross(const Vec3f& a, const Vec3f& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

} // namespace d25
