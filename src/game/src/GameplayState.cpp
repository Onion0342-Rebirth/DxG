#include "game/GameplayState.h"
#include "game/GameApp.h"
#include "game/PauseState.h"
#include "platform/Platform.h"
#include "world/Character.h"
#include "world/TileMap.h"
#include <cstdio>

namespace d25 {

namespace {
constexpr float kPlayerSpeed = 4.0f;
}

GameplayState::GameplayState(GameApp& app)
    : GameState(app)
    , app_(app)
    , world_(40, 40, 1.0f)
    , ui_(ResourceManager::fallbackFont(), 2)
    , hud_(ui_)
    , renderer_(world_, camera_, app_.resources()) {
    // 在世界中央放一个可玩角色作为基础接线（可删除/替换为正式场景）。
    world_.addCharacter(Character(Vec2f{20.f, 20.f}));
}

void GameplayState::onEnter() {
    camera_.setViewport(app_.platform().width(), app_.platform().height());
    // 固定俯视：从斜上方看向地图中部。
    camera_.configure(Vec3f{20.f, 12.f, 40.f}, Vec3f{20.f, 0.f, 20.f}, 42.f);

    if (Character* p = world_.player()) {
        p->setSpeed(kPlayerSpeed);
        p->setPos(Vec2f{20.f, 20.f});
    }
}

void GameplayState::update(float dt) {
    if (app_.input().pressed(Action::Pause)) {
        app_.states().push(std::make_unique<PauseState>(app_));
        return;
    }

    // 输入 -> 意愿方向（注意：世界中 pos.y 表示 Z）。
    Vec2f wish{0.f, 0.f};
    if (app_.input().down(Action::MoveUp))    wish.y -= 1.f; // 北 = -Z
    if (app_.input().down(Action::MoveDown))  wish.y += 1.f; // 南 = +Z
    if (app_.input().down(Action::MoveLeft))  wish.x -= 1.f; // 西 = -X
    if (app_.input().down(Action::MoveRight)) wish.x += 1.f; // 东 = +X

    if (Character* p = world_.player()) {
        p->setWishDir(wish);
    }

    world_.update(dt);
    hud_.update(dt);

    // 粗略 FPS（dt 为固定步长，这里用帧间隔更准，由 App 提供）。
    const float f = app_.frameSeconds();
    if (f > 0.f) fps_ = 1.f / f;
}

void GameplayState::render() {
    renderer_.render(app_.backBuffer(), app_.depthBuffer());
    hud_.render(app_.backBuffer(), world_.player(), fps_);
}

} // namespace d25
