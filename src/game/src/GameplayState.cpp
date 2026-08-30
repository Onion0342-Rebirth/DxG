#include "game/GameplayState.h"
#include "game/GameApp.h"
#include "game/PauseState.h"
#include "platform/Platform.h"
#include "world/player/Player.h"
#include "world/terrain/TileMap.h"
#include <cstdio>
#include <memory>

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
    // 在世界中央放一个玩家作为基础接线（可删除/替换为正式场景）。
    world_.setPlayer(std::make_unique<Player>(Vec2f{20.f, 20.f}));
}

void GameplayState::onEnter() {
    camera_.setViewport(app_.platform().width(), app_.platform().height());
    // 跟随相机：初始注视地图中央；后续每帧由玩家位置驱动（见 update）。
    Vec3f eye, target;
    followEyeTarget(Vec2f{world_.map().width() * 0.5f, world_.map().height() * 0.5f},
                    camConfig_, eye, target);
    camera_.configure(eye, target, camConfig_.fovDeg);

    if (Player* p = world_.player()) {
        p->setSpeed(kPlayerSpeed);
        p->setPos(Vec2f{20.f, 20.f});

        // 注入 8 方向 idle/walk 动画剪辑（资源未加载时指针为 nullptr，角色保持第 0 帧）。
        // 三个"朝左"方向（West/NorthWest/SouthEast）不存素材，由资源层水平镜像生成。
        const ResourceManager& res = app_.resources();
        const AnimationClip* idle[8] = {
            res.playerIdleClip(Direction::North),
            res.playerIdleClip(Direction::NorthEast),
            res.playerIdleClip(Direction::East),
            res.playerIdleClip(Direction::SouthEast),
            res.playerIdleClip(Direction::South),
            res.playerIdleClip(Direction::SouthWest),
            res.playerIdleClip(Direction::West),
            res.playerIdleClip(Direction::NorthWest),
        };
        const AnimationClip* walk[8] = {
            res.playerWalkClip(Direction::North),
            res.playerWalkClip(Direction::NorthEast),
            res.playerWalkClip(Direction::East),
            res.playerWalkClip(Direction::SouthEast),
            res.playerWalkClip(Direction::South),
            res.playerWalkClip(Direction::SouthWest),
            res.playerWalkClip(Direction::West),
            res.playerWalkClip(Direction::NorthWest),
        };
        p->setAnimClips(idle, walk);
    }
}

void GameplayState::update(float dt) {
    if (app_.input().pressed(Action::Pause)) {
        // 把跟随相机配置共享给暂停面板（VIEW 滑块调整同一份）。
        app_.states().push(std::make_unique<PauseState>(app_, &camConfig_));
        return;
    }

    // 输入 -> 意愿方向（注意：世界中 pos.y 表示 Z）。
    Vec2f wish{0.f, 0.f};
    if (app_.input().down(Action::MoveUp))    wish.y -= 1.f; // 北 = -Z
    if (app_.input().down(Action::MoveDown))  wish.y += 1.f; // 南 = +Z
    if (app_.input().down(Action::MoveLeft))  wish.x -= 1.f; // 西 = -X
    if (app_.input().down(Action::MoveRight)) wish.x += 1.f; // 东 = +X

    if (Player* p = world_.player()) {
        p->setWishDir(wish);
    }

    world_.update(dt);
    hud_.update(dt);

    // 相机跟随玩家：以玩家地面位置计算夹取后的注视点（到地图边缘停止，避免出地图外）。
    if (const Player* p = world_.player()) {
        // 视野高度联动后拉距离，保持俯角不变。
        camConfig_.back = followBackForHeight(camConfig_.height);

        const TileMap& map = world_.map();
        const float mapW = float(map.width()) * map.tileSize();
        const float mapH = float(map.height()) * map.tileSize();
        const float aspect = float(camera_.width()) / float(camera_.height());

        const Vec2f focus = computeFollowFocus(p->pos(), mapW, mapH, camConfig_, aspect);
        Vec3f eye, target;
        followEyeTarget(focus, camConfig_, eye, target);
        camera_.configure(eye, target, camConfig_.fovDeg);
    }

    // 粗略 FPS（dt 为固定步长，这里用帧间隔更准，由 App 提供）。
    const float f = app_.frameSeconds();
    if (f > 0.f) fps_ = 1.f / f;
}

void GameplayState::render() {
    renderer_.render(app_.backBuffer(), app_.depthBuffer());
    hud_.render(app_.backBuffer(), world_.player(), fps_);
}

} // namespace d25
