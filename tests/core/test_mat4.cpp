// 分类：core
// 回归验证 Mat4 视图/投影矩阵（列主序约定）：
//   - lookAt 的平移必须生效（target 点落在相机正前方、屏幕居中），防止"矩阵转置导致点跑到相机后方"；
//   - perspective 产生的 clip.w 必须等于 -z_cam。
// 纯数学，不依赖素材。
#include "framework/test_framework.h"
#include "core/Mat.h"
#include "core/Vec.h"
#include <cmath>

void runMat4Tests() {
    using namespace d25test;

    beginCase("lookAt：target 点变换到相机正前方（cam.z < 0），即平移生效");
    {
        const d25::Vec3f eye{10.f, 11.f, 22.f};
        const d25::Vec3f target{10.f, 0.f, 10.f};
        const d25::Mat4 view = d25::Mat4::lookAt(eye, target, {0.f, 1.f, 0.f});
        const d25::Vec4f cam = d25::Mat4::mul(view, {target.x, target.y, target.z, 1.f});
        // 相机看向 -Z，所以目标点的相机空间 z 必须为负（在前方）。
        CHECK(cam.z < -1.f);
        // 目标在视轴中心：相机空间 x、y 应接近 0。
        CHECK(std::fabs(cam.x) < 1e-3f);
        CHECK(std::fabs(cam.y) < 1e-3f);
        // 深度约等于 eye 到 target 的距离（≈ sqrt(11^2+12^2) ≈ 16.28）。
        const float dist = (target - eye).length();
        CHECK(std::fabs(-cam.z - dist) < 0.05f);
    }

    beginCase("lookAt：相机右侧世界点变换到 +X（右向量方向正确）");
    {
        const d25::Vec3f eye{0.f, 0.f, 5.f};
        const d25::Vec3f target{0.f, 0.f, 0.f};
        const d25::Mat4 view = d25::Mat4::lookAt(eye, target, {0.f, 1.f, 0.f});
        // 世界 +X 上的点，相机空间 x 应为正。
        const d25::Vec4f rightPt = d25::Mat4::mul(view, {1.f, 0.f, 0.f, 1.f});
        CHECK(rightPt.x > 0.5f);
        const d25::Vec4f centerPt = d25::Mat4::mul(view, {0.f, 0.f, 0.f, 1.f});
        CHECK(std::fabs(centerPt.x) < 1e-3f);
        CHECK(centerPt.z < -4.f); // 正前方
    }

    beginCase("perspective：clip.w = -z_cam（相机前方 z<0 时 w>0）");
    {
        const d25::Mat4 proj = d25::Mat4::perspective(60.f, 16.f / 9.f, 0.3f, 300.f);
        // 相机空间前方点 z = -10。
        const d25::Vec4f clip = d25::Mat4::mul(proj, {0.f, 0.f, -10.f, 1.f});
        CHECK(clip.w > 9.f && clip.w < 11.f); // w = 10
        // 视线正中点投影到 NDC 原点。
        const float ndcX = clip.x / clip.w;
        const float ndcY = clip.y / clip.w;
        CHECK(std::fabs(ndcX) < 1e-4f);
        CHECK(std::fabs(ndcY) < 1e-4f);
    }
}
