// 分类：render
// 公告板扶正与棋盘格背景测试（不依赖 SDL，不依赖素材）：
//   1. Camera::rightWorld()/upWorld() 为单位正交基向量，且与视线方向正交；
//   2. 屏幕对齐公告板：水平边投影严格水平、竖直边投影严格竖直——模拟地图边缘夹取
//      （玩家偏离视线轴）时角色也不歪斜（旧"圆柱公告板 + 世界竖直边"在此情形会歪）；
//   3. 屏幕对齐公告板在俯视下屏幕高宽比恒等于世界高宽比（正方形帧不被压扁）；
//   4. 棋盘格地面：相邻瓦片明暗交替，中心采样颜色明显不同。
#include "framework/test_framework.h"
#include "render/Camera.h"
#include "render/SceneRenderer.h"
#include "core/PixelBuffer.h"
#include "core/DepthBuffer.h"
#include "core/Vec.h"
#include "world/World.h"
#include "res/ResourceManager.h"
#include <cmath>

namespace {

using namespace d25;

float lengthOf(const Vec3f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float dot3(const Vec3f& a, const Vec3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// 投影世界点到屏幕坐标（失败返回 false）。
bool projectXY(const Camera& cam, const Vec3f& p, float& sx, float& sy) {
    ScreenVertex sv;
    if (!cam.projectWorld(p, sv)) return false;
    sx = sv.x;
    sy = sv.y;
    return true;
}

} // namespace

void runBillboardBoardTests() {
    using namespace d25test;

    SECTION("render");

    // ---------- 1. 相机世界系基向量 ----------
    beginCase("相机右/上基向量为单位向量、互相正交且与视线正交");
    {
        // 约 45° 俯角机位（与 FollowCam 默认一致）。
        Camera cam;
        cam.setViewport(480, 360);
        cam.configure(Vec3f{20.f, 9.f, 29.f}, Vec3f{20.f, 0.f, 20.f}, 42.f);

        const Vec3f r = cam.rightWorld();
        const Vec3f u = cam.upWorld();
        const Vec3f fwd = (Vec3f{20.f, 0.f, 20.f} - cam.eye()).normalized();

        CHECK(std::fabs(lengthOf(r) - 1.f) < 1e-4f);
        CHECK(std::fabs(lengthOf(u) - 1.f) < 1e-4f);
        CHECK(std::fabs(dot3(r, u)) < 1e-4f);       // 右 × 上 正交
        CHECK(std::fabs(dot3(r, fwd)) < 1e-4f);     // 右 × 视线 正交
        CHECK(std::fabs(dot3(u, fwd)) < 1e-4f);     // 上 × 视线 正交
        // 俯视约 45°：相机上向量朝后上方，世界 y 分量约为 sin(45°)=0.707。
        CHECK(std::fabs(u.y - 0.7071f) < 0.02f);
        // 相机无滚转：右向量严格水平（y 分量为 0）。
        CHECK(std::fabs(r.y) < 1e-4f);
    }

    beginCase("相机沿 -Z 平视时右=(1,0,0)、上=(0,1,0)");
    {
        Camera cam;
        cam.setViewport(480, 360);
        cam.configure(Vec3f{0.f, 0.f, 10.f}, Vec3f{0.f, 0.f, 0.f}, 42.f);
        const Vec3f r = cam.rightWorld();
        const Vec3f u = cam.upWorld();
        CHECK(std::fabs(r.x - 1.f) < 1e-4f);
        CHECK(std::fabs(r.y) < 1e-4f && std::fabs(r.z) < 1e-4f);
        CHECK(std::fabs(u.y - 1.f) < 1e-4f);
        CHECK(std::fabs(u.x) < 1e-4f && std::fabs(u.z) < 1e-4f);
    }

    // ---------- 2. 公告板屏幕对齐（边缘不歪斜） ----------
    // 构造一个屏幕对齐公告板并返回四角投影后的屏幕坐标（corners: 0=左上 1=右上 2=右下 3=左下）。
    auto projectBoard = [&](const Camera& cam, const Vec3f& base, float w, float h,
                            float sx[4], float sy[4]) -> bool {
        Vec3f corners[4];
        SceneRenderer::billboardCorners(cam.rightWorld(), cam.upWorld(), base, w, h, corners);
        bool ok = true;
        for (int i = 0; i < 4; ++i)
            ok &= projectXY(cam, corners[i], sx[i], sy[i]);
        return ok;
    };

    beginCase("地图边缘夹取机位下，公告板水平边严格水平、竖直边严格竖直");
    {
        // 模拟边缘：注视点（20,20）夹在地图内，玩家被挤到屏幕侧边（12,16）。
        Camera cam;
        cam.setViewport(480, 360);
        cam.configure(Vec3f{20.f, 9.f, 29.f}, Vec3f{20.f, 0.f, 20.f}, 42.f);

        float sx[4], sy[4];
        CHECK(projectBoard(cam, Vec3f{12.f, 0.f, 16.f}, 0.8f, 0.8f, sx, sy));

        // 顶边（左上-右上）、底边（左下-右下）投影后 y 相等 -> 严格水平。
        CHECK(std::fabs(sy[0] - sy[1]) < 0.5f);
        CHECK(std::fabs(sy[3] - sy[2]) < 0.5f);
        // 左边（左上-左下）、右边（右上-右下）投影后 x 相等 -> 严格竖直。
        CHECK(std::fabs(sx[0] - sx[3]) < 0.5f);
        CHECK(std::fabs(sx[1] - sx[2]) < 0.5f);
    }

    // ---------- 3. 俯视下不压扁 ----------
    beginCase("45° 俯角下正方形公告板屏幕高宽比恒为 1（不被透视压扁）");
    {
        Camera cam;
        cam.setViewport(480, 360);
        cam.configure(Vec3f{10.f, 9.f, 19.f}, Vec3f{10.f, 0.f, 10.f}, 42.f);

        // 玩家在屏幕中央。
        {
            float sx[4], sy[4];
            CHECK(projectBoard(cam, Vec3f{10.f, 0.f, 10.f}, 0.8f, 0.8f, sx, sy));
            const float screenW = std::fabs(sx[2] - sx[3]); // 右下 - 左下
            const float screenH = std::fabs(sy[3] - sy[0]); // 左下 - 左上
            const float ratio = screenH / screenW;
            std::printf("    [info] 中央正方形公告板屏幕高宽比 = %.3f\n", ratio);
            CHECK(std::fabs(ratio - 1.f) < 0.05f);
        }
        // 玩家偏离到屏幕侧边（边缘夹取情形）：高宽比仍应接近 1，不歪不扁。
        {
            float sx[4], sy[4];
            CHECK(projectBoard(cam, Vec3f{6.f, 0.f, 8.f}, 0.8f, 0.8f, sx, sy));
            const float screenW = std::fabs(sx[2] - sx[3]);
            const float screenH = std::fabs(sy[3] - sy[0]);
            const float ratio = screenH / screenW;
            std::printf("    [info] 侧边正方形公告板屏幕高宽比 = %.3f\n", ratio);
            CHECK(std::fabs(ratio - 1.f) < 0.05f);
        }
    }

    // ---------- 4. 棋盘格地面 ----------
    beginCase("棋盘格：相邻瓦片中心像素颜色明显不同（明暗交替）");
    {
        // 默认构造 ResourceManager（不加载素材）：terrain() 返回 nullptr，走兜底色 + 棋盘明暗。
        World world(12, 12, 1.0f);
        ResourceManager res;

        Camera cam;
        cam.setViewport(480, 360);
        cam.configure(Vec3f{6.f, 9.f, 15.f}, Vec3f{6.f, 0.f, 6.f}, 42.f);

        PixelBuffer fb(480, 360);
        DepthBuffer db; db.resize(480, 360);
        SceneRenderer renderer(world, cam, res);
        renderer.render(fb, db);

        // 取相邻两个瓦片 (5,5) 与 (6,5) 的中心点世界坐标，投影到屏幕采样。
        auto sampleTile = [&](int tx, int tz, Color& out) -> bool {
            const Vec3f center{tx + 0.5f, 0.f, tz + 0.5f};
            ScreenVertex sv;
            if (!cam.projectWorld(center, sv)) return false;
            const int x = int(std::lround(sv.x));
            const int y = int(std::lround(sv.y));
            if (x < 1 || y < 1 || x >= fb.width() - 1 || y >= fb.height() - 1) return false;
            out = fb.get(x, y);
            return true;
        };

        Color a, b;
        bool ok = sampleTile(5, 5, a) && sampleTile(6, 5, b);
        CHECK(ok);
        const int dr = int(a.r) - int(b.r);
        const int dg = int(a.g) - int(b.g);
        const int dbc = int(a.b) - int(b.b);
        const int dist2 = dr * dr + dg * dg + dbc * dbc;
        std::printf("    [info] 相邻瓦片颜色 (%d,%d,%d) vs (%d,%d,%d)，色差平方 = %d\n",
                    a.r, a.g, a.b, b.r, b.g, b.b, dist2);
        // 暗格为亮格的 0.82 倍，草色 g 分量差约 28，色差平方应远大于阈值。
        CHECK(dist2 > 200);

        // 对角瓦片 (tx+tz 奇偶相同) 应为同色（棋盘周期为 2）。
        Color c;
        bool ok2 = sampleTile(6, 6, c);
        CHECK(ok2);
        const int sameDist2 =
            (int(a.r) - int(c.r)) * (int(a.r) - int(c.r)) +
            (int(a.g) - int(c.g)) * (int(a.g) - int(c.g)) +
            (int(a.b) - int(c.b)) * (int(a.b) - int(c.b));
        std::printf("    [info] 对角同奇偶瓦片色差平方 = %d\n", sameDist2);
        CHECK(sameDist2 < 30);
    }
}
