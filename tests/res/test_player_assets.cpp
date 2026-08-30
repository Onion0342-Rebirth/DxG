// 分类：res
// 验证 ResourceManager::loadPlayerRobot：精灵表帧数/尺寸、三对水平镜像
// （West←East、NorthWest←NorthEast、SouthEast←SouthWest）、8 方向动画剪辑参数。
// 需要真实素材；定位不到 assets 时按 SKIP 处理。
#include "framework/test_framework.h"
#include "res/ResourceManager.h"
#include "render/SpriteSheet.h"
#include "anim/AnimationClip.h"
#include "world/character/Direction.h"
#include <string>

namespace {
// 比较两帧是否满足 "a 是 b 的水平镜像"：a(x,y) == b(w-1-x,y)。
bool isHorizontalMirror(const d25::Sprite& a, const d25::Sprite& b) {
    if (a.width() != b.width() || a.height() != b.height()) return false;
    const int w = a.width(), h = a.height();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const d25::Color ca = a.pixel(x, y);
            const d25::Color cb = b.pixel(w - 1 - x, y);
            if (ca.r != cb.r || ca.g != cb.g || ca.b != cb.b || ca.a != cb.a) return false;
        }
    }
    return true;
}

// 8 方向按枚举顺序（N/NE/E/SE/S/SW/W/NW），每方向 6 帧。
const d25::Direction kDirs[8] = {
    d25::Direction::North,     d25::Direction::NorthEast,
    d25::Direction::East,      d25::Direction::SouthEast,
    d25::Direction::South,     d25::Direction::SouthWest,
    d25::Direction::West,      d25::Direction::NorthWest};
}

void runPlayerAssetsTests() {
    using namespace d25test;

    const std::string assets = findAssetsDir();
    if (assets.empty()) {
        SKIP_REASON("未找到 assets 目录，跳过资源加载用例");
        return;
    }

    d25::ResourceManager rm;
    std::string err;
    beginCase("loadPlayerRobot 加载成功");
    bool ok = rm.loadPlayerRobot(assets, &err);
    CHECK(ok);
    if (!ok) {
        std::printf("    错误信息: %s\n", err.c_str());
        return;
    }

    beginCase("玩家精灵表为 48 帧、每帧 32x32");
    const d25::SpriteSheet* sheet = rm.playerSheet();
    CHECK(sheet != nullptr);
    if (sheet) {
        CHECK(sheet->frameCount() == 48);
        bool all32 = true;
        for (int i = 0; i < sheet->frameCount(); ++i) {
            const d25::Sprite& f = sheet->frame(i);
            if (f.width() != 32 || f.height() != 32) all32 = false;
        }
        CHECK(all32);
    }

    // 三对镜像：镜像方向 6 帧 == 源方向 6 帧的水平镜像。
    // 帧基址 = (int)Direction * 6：NE=6、SE=18、E=12、SW=30、W=36、NW=42。
    struct MirrorCase { const char* name; d25::Direction mirror; d25::Direction source; };
    const MirrorCase mirrorCases[3] = {
        {"West(左) == East(右)",        d25::Direction::West,      d25::Direction::East},
        {"NorthWest(左上) == NorthEast(右上)", d25::Direction::NorthWest, d25::Direction::NorthEast},
        {"SouthEast(右下) == SouthWest(左下)", d25::Direction::SouthEast, d25::Direction::SouthWest},
    };
    for (const MirrorCase& mc : mirrorCases) {
        beginCase(std::string("镜像：") + mc.name + " 的水平镜像");
        if (sheet && sheet->frameCount() == 48) {
            bool mirrorOk = true;
            for (int f = 0; f < 6; ++f) {
                const d25::Sprite& src = sheet->frame(int(mc.source) * 6 + f);
                const d25::Sprite& dst = sheet->frame(int(mc.mirror) * 6 + f);
                if (!isHorizontalMirror(dst, src)) { mirrorOk = false; break; }
            }
            CHECK(mirrorOk);
        }
    }

    beginCase("walk 剪辑：8 方向各 6 帧、6fps（放慢）、循环，基址 d*6");
    {
        bool clipOk = true;
        for (int d = 0; d < 8; ++d) {
            const d25::AnimationClip* w = rm.playerWalkClip(kDirs[d]);
            if (!w) { clipOk = false; continue; }
            if (w->firstFrame != d * 6 || w->frameCount != 6 || !w->loop) clipOk = false;
            // fps 近似比较（6.0）。
            if (w->fps < 5.9f || w->fps > 6.1f) clipOk = false;
        }
        CHECK(clipOk);
    }

    beginCase("idle 剪辑：8 方向各 6 帧、2fps（缓慢待机循环）、基址与 walk 一致");
    {
        bool clipOk = true;
        for (int d = 0; d < 8; ++d) {
            const d25::AnimationClip* i = rm.playerIdleClip(kDirs[d]);
            if (!i) { clipOk = false; continue; }
            // idle 也用满 6 帧循环，只是 fps 更慢（2.0）。
            if (i->firstFrame != d * 6 || i->frameCount != 6 || !i->loop) clipOk = false;
            if (i->fps < 1.9f || i->fps > 2.1f) clipOk = false;
        }
        CHECK(clipOk);
    }

    beginCase("loadPlayerRobot 对无效目录返回 false 且不崩溃");
    {
        d25::ResourceManager rm2;
        std::string e2;
        bool bad = rm2.loadPlayerRobot("__no_such_assets_dir__", &e2);
        CHECK(!bad);
        CHECK(rm2.playerSheet() == nullptr);
    }
}
