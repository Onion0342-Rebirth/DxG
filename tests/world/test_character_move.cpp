// 分类：world
// 回归验证角色移动（Character 基类能力，玩家/NPC/怪物共用）：WASD 八方向位移、
// 朝向更新（4 个纯轴 + 4 个斜向）、斜向归一化（不加速）、撞水/阻挡回退（轴分离滑动）、
// 地图边界阻挡。纯逻辑，不依赖素材。直接驱动 Character::update(dt, map)。
#include "framework/test_framework.h"
#include "world/World.h"
#include "world/character/Character.h"
#include "world/terrain/TileMap.h"
#include "anim/AnimationClip.h"
#include "core/Vec.h"
#include <cmath>

namespace {
constexpr float kEps = 1e-3f;

float dist(float x1, float z1, float x2, float z2) {
    float dx = x2 - x1, dz = z2 - z1;
    return std::sqrt(dx * dx + dz * dz);
}
}

void runCharacterMoveTests() {
    using namespace d25test;

    beginCase("向下(MoveDown/S)：+Z 方向移动，朝向 South");
    {
        d25::World world(10, 10, 1.0f);
        d25::Character p{d25::Vec2f{5.f, 5.f}};
        p.setSpeed(4.0f);
        p.setWishDir(d25::Vec2f{0.f, 1.f}); // 南 = +Z
        p.update(0.25f, world.map());       // 0.25 秒
        CHECK(std::fabs(p.pos().x - 5.f) < kEps);
        CHECK(std::fabs(p.pos().y - 6.f) < kEps); // 5 + 4*0.25 = 6
        CHECK(p.facing() == d25::Direction::South);
        CHECK(p.moving());
    }

    beginCase("向上(W)：-Z；向左(A)：-X 朝 West；向右(D)：+X 朝 East");
    {
        d25::World world(20, 20, 1.0f);
        d25::Character p{d25::Vec2f{10.f, 10.f}};
        p.setSpeed(4.0f);

        p.setWishDir(d25::Vec2f{0.f, -1.f}); // 上 W
        p.update(0.1f, world.map());
        CHECK(p.facing() == d25::Direction::North);
        CHECK(p.pos().y < 10.f - kEps);

        p.setWishDir(d25::Vec2f{-1.f, 0.f}); // 左 A
        p.update(0.1f, world.map());
        CHECK(p.facing() == d25::Direction::West);
        float xAfterLeft = p.pos().x;

        p.setWishDir(d25::Vec2f{1.f, 0.f});  // 右 D
        p.update(0.1f, world.map());
        CHECK(p.facing() == d25::Direction::East);
        CHECK(p.pos().x > xAfterLeft);
    }

    beginCase("斜向移动已归一化（对角线不加速），等分量(1,1) 朝向 SouthEast");
    {
        d25::World world(40, 40, 1.0f);
        d25::Character p{d25::Vec2f{20.f, 20.f}};
        p.setSpeed(4.0f);
        p.setWishDir(d25::Vec2f{1.f, 1.f}); // 右下对角（未归一化长度 sqrt2）
        p.update(1.0f, world.map());
        // 归一化后每轴速度 = 4/sqrt2，总位移应等于 speed*dt = 4。
        const float moved = dist(20.f, 20.f, p.pos().x, p.pos().y);
        CHECK(std::fabs(moved - 4.0f) < 0.05f);
        // 等分量 => 正好 45°，量化为 SouthEast（右下）。
        CHECK(p.facing() == d25::Direction::SouthEast);
    }

    beginCase("八方向朝向量化：四个斜向意愿对应 NE/SE/SW/NW");
    {
        d25::World world(40, 40, 1.0f);
        struct Case { d25::Vec2f wish; d25::Direction expect; };
        const Case cases[4] = {
            {{ 1.f, -1.f}, d25::Direction::NorthEast}, // 右上
            {{ 1.f,  1.f}, d25::Direction::SouthEast}, // 右下
            {{-1.f,  1.f}, d25::Direction::SouthWest}, // 左下
            {{-1.f, -1.f}, d25::Direction::NorthWest}, // 左上
        };
        for (const Case& c : cases) {
            d25::Character p{d25::Vec2f{20.f, 20.f}};
            p.setWishDir(c.wish);
            p.update(0.05f, world.map());
            CHECK(p.facing() == c.expect);
        }
    }

    beginCase("近轴方向不误判为斜向（主导分量足够大时落到纯轴方向）");
    {
        d25::World world(40, 40, 1.0f);
        // x 分量远大于 z：atan2 角度约 11°，应量化到 East 而非 SouthEast。
        d25::Character p{d25::Vec2f{20.f, 20.f}};
        p.setWishDir(d25::Vec2f{1.f, 0.2f});
        p.update(0.05f, world.map());
        CHECK(p.facing() == d25::Direction::East);
    }

    beginCase("无输入：静止，不播放移动（animFrame 为 idle 首帧）");
    {
        d25::World world(20, 20, 1.0f);
        d25::Character p{d25::Vec2f{10.f, 10.f}};
        p.setWishDir(d25::Vec2f{0.f, 0.f});
        p.update(0.5f, world.map());
        CHECK(!p.moving());
        CHECK(std::fabs(p.pos().x - 10.f) < kEps);
        CHECK(std::fabs(p.pos().y - 10.f) < kEps);
    }

    beginCase("撞水阻挡：朝水移动时该轴回退（不可进入 Water）");
    {
        d25::World world(20, 20, 1.0f);
        d25::TileMap& map = world.map();
        // 角色前方（+Z 方向，z≈12~13 一带）铺一条水墙。
        for (int tx = 8; tx <= 14; ++tx) {
            map.at(tx, 13).terrain = d25::Terrain::Water;
        }
        d25::Character p{d25::Vec2f{10.f, 10.f}};
        p.setSpeed(4.0f);
        p.setWishDir(d25::Vec2f{0.f, 1.f}); // 向南（朝水墙）
        for (int i = 0; i < 20; ++i) p.update(0.05f, map); // 累计 1 秒，足够撞上
        // 角色半径 0.28，z 不应越过水墙前沿（13 - 0.28 ≈ 12.72）。
        CHECK(p.pos().y < 13.f - p.radius() - kEps);
    }

    beginCase("轴分离滑动：X 被挡时 Z 仍可移动（贴墙滑动）");
    {
        d25::World world(20, 20, 1.0f);
        d25::TileMap& map = world.map();
        // 在角色 +X 方向立一堵水墙（x≈12），Z 方向保持草地。
        for (int tz = 8; tz <= 16; ++tz) {
            map.at(12, tz).terrain = d25::Terrain::Water;
        }
        d25::Character p{d25::Vec2f{10.f, 10.f}};
        p.setSpeed(4.0f);
        p.setWishDir(d25::Vec2f{1.f, 1.f}); // 东南对角：X 撞墙、Z 应滑动
        for (int i = 0; i < 20; ++i) p.update(0.05f, map);
        // X 被墙挡住（不越过 12-radius），Z 仍向南滑动（明显推进）。
        CHECK(p.pos().x < 12.f - p.radius() - kEps);
        CHECK(p.pos().y > 10.5f);
    }

    beginCase("地图边界阻挡：试图走出地图被夹回");
    {
        d25::World world(10, 10, 1.0f);
        d25::Character p{d25::Vec2f{0.5f, 0.5f}};
        p.setSpeed(8.0f);
        p.setWishDir(d25::Vec2f{-1.f, -1.f}); // 朝地图外角冲
        for (int i = 0; i < 30; ++i) p.update(0.05f, world.map());
        // 越界视为阻挡，位置不应变成负数（被挡在地图内）。
        CHECK(p.pos().x >= -kEps);
        CHECK(p.pos().y >= -kEps);
    }

    beginCase("动画节奏：走路切帧较快，静止(idle)也缓慢切帧且更慢");
    {
        // 纯逻辑用例（不依赖 PNG 素材）：手工构造 walk=6fps / idle=2fps、均 6 帧循环的剪辑。
        d25::AnimationClip walkClips[8];
        d25::AnimationClip idleClips[8];
        for (int d = 0; d < 8; ++d) {
            walkClips[d] = d25::AnimationClip{d * 6, 6, 6.0f, true}; // 6fps：6 帧一轮 1.0 秒
            idleClips[d] = d25::AnimationClip{d * 6, 6, 2.0f, true}; // 2fps：6 帧一轮 3.0 秒
        }
        const d25::AnimationClip* idlePtrs[8] = {
            &idleClips[0], &idleClips[1], &idleClips[2], &idleClips[3],
            &idleClips[4], &idleClips[5], &idleClips[6], &idleClips[7]};
        const d25::AnimationClip* walkPtrs[8] = {
            &walkClips[0], &walkClips[1], &walkClips[2], &walkClips[3],
            &walkClips[4], &walkClips[5], &walkClips[6], &walkClips[7]};

        d25::World world(40, 40, 1.0f);
        d25::Character p{d25::Vec2f{20.f, 20.f}};
        p.setSpeed(4.0f);
        p.setAnimClips(idlePtrs, walkPtrs);

        // 统计一段时间内 animFrame() 发生变化的次数（跨帧即 +1，循环 wrap 也算变化）。
        auto countFrameChanges = [&](int steps, float dt) {
            int changes = 0;
            int last = p.animFrame();
            for (int i = 0; i < steps; ++i) {
                p.update(dt, world.map());
                const int cur = p.animFrame();
                if (cur != last) { ++changes; last = cur; }
            }
            return changes;
        };

        // 移动 2 秒（40 x 0.05s）：6fps 下约切 12 帧，应明显在动。
        p.setWishDir(d25::Vec2f{0.f, 1.f});
        const int walkChanges = countFrameChanges(40, 0.05f);
        CHECK(walkChanges >= 8);

        // 静止 2 秒：idle 2fps 下约切 4 帧——既不是 0（不再僵住），又明显比走路慢。
        p.setWishDir(d25::Vec2f{0.f, 0.f});
        p.update(0.05f, world.map()); // moving_ 在 update 内按 wish_ 刷新，先推进一步确认静止
        CHECK(!p.moving());
        const int idleChanges = countFrameChanges(39, 0.05f); // 连同上一步共 2 秒
        CHECK(idleChanges > 0);
        CHECK(idleChanges < walkChanges);
    }
}
