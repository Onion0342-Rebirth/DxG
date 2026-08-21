#pragma once
#include "core/Color.h"
#include "game/GameState.h"

namespace d25 {

class GameApp;
class UIRenderer;
class PixelBuffer;

// 暂停状态：画面定格（不 update 下层），居中显示"PAUSED"面板。
// 按 Pause/Confirm/Cancel 恢复，按 Quit 退出游戏。
class PauseState : public GameState {
public:
    explicit PauseState(GameApp& app);

    void onEnter() override;
    void update(float dt) override;
    void render() override;

    // 供测试/无头复用：在给定缓冲上画暂停面板。
    static void drawOverlay(UIRenderer& ui, PixelBuffer& buf);

private:
    GameApp& app_;
};

} // namespace d25
