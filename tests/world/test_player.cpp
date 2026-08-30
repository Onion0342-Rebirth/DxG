// 分类：world
// 验证 world 子包重组后的玩家归属：
//   - Player 定义在 world/player，且 is-a Character（可直接使用移动/动画接口）；
//   - World 通过 setPlayer 注入并持有玩家，player() 返回该玩家；
//   - World::update 会推进玩家移动；未注入玩家时 player() 为 nullptr 且 update 不崩溃；
//   - 非玩家角色仍由 addCharacter/characters() 持有，且与玩家同时被推进。
// 纯逻辑，不依赖素材。
#include "framework/test_framework.h"
#include "world/World.h"
#include "world/player/Player.h"
#include "world/character/Character.h"
#include <memory>
#include <type_traits>
#include <cmath>

namespace {
constexpr float kEps = 1e-3f;
}

void runPlayerTests() {
    using namespace d25test;

    beginCase("Player 定义于 world/player，且公开继承 Character");
    {
        static_assert(std::is_base_of<d25::Character, d25::Player>::value,
                      "Player 必须公开继承 Character");
        static_assert(std::is_convertible<d25::Player*, d25::Character*>::value,
                      "Player* 必须可隐式转换为 Character*（供渲染/HUD 使用）");
        d25::Player player(d25::Vec2f{3.f, 4.f});
        CHECK(std::fabs(player.pos().x - 3.f) < kEps);
        CHECK(std::fabs(player.pos().y - 4.f) < kEps);
    }

    beginCase("未注入玩家时 player() 为 nullptr，update 不崩溃");
    {
        d25::World world(10, 10, 1.0f);
        CHECK(world.player() == nullptr);
        world.update(0.1f); // 不应崩溃
        CHECK(world.player() == nullptr);
    }

    beginCase("setPlayer 注入后 player() 返回该玩家（同一对象）");
    {
        d25::World world(10, 10, 1.0f);
        auto player = std::make_unique<d25::Player>(d25::Vec2f{5.f, 5.f});
        d25::Player* raw = player.get();
        world.setPlayer(std::move(player));
        CHECK(world.player() == raw);
    }

    beginCase("World::update 推进玩家移动（输入意愿 -> 位移）");
    {
        d25::World world(20, 20, 1.0f);
        world.setPlayer(std::make_unique<d25::Player>(d25::Vec2f{10.f, 10.f}));
        d25::Player* p = world.player();
        p->setSpeed(4.0f);
        p->setWishDir(d25::Vec2f{0.f, 1.f}); // 南 = +Z
        world.update(0.25f);                  // 0.25 秒 -> z 应前进 1 米
        CHECK(std::fabs(p->pos().x - 10.f) < kEps);
        CHECK(std::fabs(p->pos().y - 11.f) < kEps);
        CHECK(p->facing() == d25::Direction::South);
    }

    beginCase("玩家与非玩家角色共存：两者都被 World::update 推进");
    {
        d25::World world(40, 40, 1.0f);
        world.setPlayer(std::make_unique<d25::Player>(d25::Vec2f{20.f, 20.f}));
        world.addCharacter(d25::Character(d25::Vec2f{5.f, 5.f})); // 一个 NPC
        CHECK(world.characters().size() == 1u);

        world.player()->setSpeed(4.0f);
        world.player()->setWishDir(d25::Vec2f{0.f, 1.f});
        d25::Character& npc = world.characters().front();
        npc.setSpeed(4.0f);
        npc.setWishDir(d25::Vec2f{1.f, 0.f}); // NPC 向东

        world.update(0.25f);

        // 玩家向南（+Z）。
        CHECK(std::fabs(world.player()->pos().y - 21.f) < kEps);
        // NPC 向东（+X）。
        CHECK(std::fabs(npc.pos().x - 6.f) < kEps);
        CHECK(std::fabs(npc.pos().y - 5.f) < kEps);
    }
}
