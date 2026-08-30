// 分类：render
// 离屏渲染冒烟测试（不依赖 SDL）：加载角色素材 -> 构造角色并朝八个方向 ->
// 用 SceneRenderer 把场景软件光栅化到 PixelBuffer -> 统计非背景像素，
// 验证角色像素确实被画出来；并导出 PPM 截图（build/ 下），方便人工查看。
#include "framework/test_framework.h"
#include "res/ResourceManager.h"
#include "world/World.h"
#include "world/player/Player.h"
#include <memory>
#include "render/Camera.h"
#include "render/SceneRenderer.h"
#include "core/PixelBuffer.h"
#include "core/DepthBuffer.h"
#include "core/ImageIO.h"
#include "core/Color.h"
#include "core/Vec.h"
#include <string>

namespace {
// 统计两帧之间"颜色明显不同"的像素数（用于差分判定角色是否被画出来，不受雾/天空颜色影响）。
long countDiffPixels(const d25::PixelBuffer& a, const d25::PixelBuffer& b) {
    long diff = 0;
    const int n = a.width() * a.height();
    for (int i = 0; i < n; ++i) {
        const d25::Color ca = a.data()[i];
        const d25::Color cb = b.data()[i];
        const int dr = int(ca.r) - cb.r;
        const int dg = int(ca.g) - cb.g;
        const int db = int(ca.b) - cb.b;
        if (dr * dr + dg * dg + db * db > 25 * 25) ++diff; // 颜色差阈值
    }
    return diff;
}

// 渲染一帧：withActor=true 时在 (10,10) 放一个朝向 facing 的角色；false 时只有空场景。
// 已加载素材的 ResourceManager 由调用方传入（角色剪辑据此注入）。
void renderScene(const d25::ResourceManager& res, d25::Direction facing, bool withActor,
                 d25::PixelBuffer& fb, d25::DepthBuffer& db) {
    using namespace d25;
    World world(20, 20, 1.0f);

    if (withActor) {
        world.setPlayer(std::make_unique<Player>(Vec2f{10.f, 10.f}));
        Player* p = world.player();
        const AnimationClip* idle[8] = {
            res.playerIdleClip(Direction::North),     res.playerIdleClip(Direction::NorthEast),
            res.playerIdleClip(Direction::East),      res.playerIdleClip(Direction::SouthEast),
            res.playerIdleClip(Direction::South),     res.playerIdleClip(Direction::SouthWest),
            res.playerIdleClip(Direction::West),      res.playerIdleClip(Direction::NorthWest)};
        const AnimationClip* walk[8] = {
            res.playerWalkClip(Direction::North),     res.playerWalkClip(Direction::NorthEast),
            res.playerWalkClip(Direction::East),      res.playerWalkClip(Direction::SouthEast),
            res.playerWalkClip(Direction::South),     res.playerWalkClip(Direction::SouthWest),
            res.playerWalkClip(Direction::West),      res.playerWalkClip(Direction::NorthWest)};
        p->setAnimClips(idle, walk);

        Vec2f wish{0.f, 0.f};
        switch (facing) {
            case Direction::North:     wish = { 0.f, -1.f}; break;
            case Direction::NorthEast: wish = { 1.f, -1.f}; break;
            case Direction::East:      wish = { 1.f,  0.f}; break;
            case Direction::SouthEast: wish = { 1.f,  1.f}; break;
            case Direction::South:     wish = { 0.f,  1.f}; break;
            case Direction::SouthWest: wish = {-1.f,  1.f}; break;
            case Direction::West:      wish = {-1.f,  0.f}; break;
            case Direction::NorthWest: wish = {-1.f, -1.f}; break;
            default: break;
        }
        p->setWishDir(wish);
        world.update(0.016f); // 更新朝向并切到对应剪辑
    }

    Camera cam;
    cam.setViewport(fb.width(), fb.height());
    cam.configure(Vec3f{10.f, 11.f, 22.f}, Vec3f{10.f, 0.f, 10.f}, 42.f);
    SceneRenderer renderer(world, cam, res);
    renderer.render(fb, db);
}
}

void runRenderSmokeTests() {
    using namespace d25test;

    const std::string assets = findAssetsDir();
    if (assets.empty()) {
        SKIP_REASON("未找到 assets 目录，跳过离屏渲染用例");
        return;
    }

    d25::ResourceManager res;
    std::string err;
    beginCase("资源加载成功（离屏渲染前提）");
    CHECK(res.loadPlayerRobot(assets, &err));

    // 基线帧：相同相机、相同世界，但不放角色。
    d25::PixelBuffer base(480, 360);
    {
        d25::DepthBuffer db; db.resize(480, 360);
        renderScene(res, d25::Direction::South, /*withActor*/false, base, db);
    }

    struct Shot { d25::Direction dir; const char* name; };
    const Shot shots[8] = {
        {d25::Direction::South,     "shot_south"},
        {d25::Direction::SouthEast, "shot_southeast"},
        {d25::Direction::East,      "shot_east"},
        {d25::Direction::NorthEast, "shot_northeast"},
        {d25::Direction::North,     "shot_north"},
        {d25::Direction::NorthWest, "shot_northwest"},
        {d25::Direction::West,      "shot_west"},
        {d25::Direction::SouthWest, "shot_southwest"},
    };

    d25::PixelBuffer prevFrame;
    bool allHaveActor = true;
    for (int si = 0; si < 8; ++si) {
        const Shot& s = shots[si];
        d25::PixelBuffer fb(480, 360);
        d25::DepthBuffer db; db.resize(480, 360);
        renderScene(res, s.dir, /*withActor*/true, fb, db);
        d25::writePPM(fb, std::string(s.name) + ".ppm", nullptr);

        const long diffVsBase = countDiffPixels(base, fb);
        std::printf("    [info] %s 相对空场景差异像素 = %ld\n", s.name, diffVsBase);

        // 角色公告板应在空场景上留下明显差异；阈值取宽松下限（East/West 素材横向更窄）。
        beginCase(std::string("离屏渲染：朝向 ") + s.name + " 相对空场景有角色像素");
        if (!(diffVsBase > 80)) allHaveActor = false;
        CHECK(diffVsBase > 80);

        if (si > 0) {
            // 相邻朝向（尤其 East vs West）应有可见差异，证明确实按朝向取了不同帧。
            const long diffVsPrev = countDiffPixels(prevFrame, fb);
            std::printf("    [info] %s 与前一朝向差异 = %ld\n", s.name, diffVsPrev);
            beginCase(std::string("朝向 ") + s.name + " 与前一朝向画面不同");
            CHECK(diffVsPrev > 30);
        }
        prevFrame = fb;
    }
    beginCase("八个朝向都成功绘制角色像素");
    CHECK(allHaveActor);
}
