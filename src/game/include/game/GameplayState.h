#pragma once
#include "core/Vec.h"
#include "game/GameState.h"
#include "world/World.h"
#include "render/Camera.h"
#include "render/FollowCam.h"
#include "render/SceneRenderer.h"
#include "ui/UIRenderer.h"
#include "ui/HUD.h"

namespace d25 {

class GameApp;

// 基础玩法状态：
//   - 一个空的默认世界（40x40 草地）+ 一个可 WASD 移动的角色
//   - 相机跟随玩家，到地图边缘时夹取停止（不会看到地图外）
//   - SceneRenderer 渲染世界，HUD 叠提示/FPS/坐标
// 这是框架自带的"可玩空场景"，具体关卡/内容由在此基础上扩展。
class GameplayState : public GameState {
public:
    explicit GameplayState(GameApp& app);

    void onEnter() override;
    void update(float dt) override;
    void render() override;

    // 跟随相机参数（暂停菜单的 VIEW 滑块共享同一份，调整后恢复游戏即生效）。
    FollowCamConfig& camConfig() { return camConfig_; }

private:
    GameApp& app_;
    World world_;
    Camera camera_;
    FollowCamConfig camConfig_;
    UIRenderer ui_;
    HUD hud_;
    SceneRenderer renderer_;
    float fps_ = 60.f;
};

} // namespace d25
