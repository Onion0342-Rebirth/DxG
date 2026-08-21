#pragma once
#include "core/Game.h"
#include "core/PixelBuffer.h"
#include "core/DepthBuffer.h"
#include "input/InputManager.h"
#include "res/ResourceManager.h"
#include "game/GameState.h"

namespace d25 {

class Platform;
class GameplayState;
class PauseState;

// 组合根：拥有平台引用、帧缓冲、输入、资源与状态栈，并把它们接到 Game 主循环。
// 不直接依赖 SDL（平台由构造时注入）。
class GameApp : public Game {
public:
    explicit GameApp(Platform& platform);
    ~GameApp() override;

    Platform& platform() { return platform_; }
    InputManager& input() { return input_; }
    ResourceManager& resources() { return resources_; }
    const ResourceManager& resources() const { return resources_; }

    PixelBuffer& backBuffer() { return backBuffer_; }
    DepthBuffer& depthBuffer() { return depthBuffer_; }
    StateStack& states() { return states_; }

    // 每帧实际耗时（秒），供 HUD/动画使用。
    float frameSeconds() const { return frameSeconds_; }

protected:
    void onInit() override;
    void onPreFrame(float dt) override;
    void onUpdate(float dt) override;
    void onRender() override;
    void onShutdown() override;

private:
    Platform& platform_;
    PixelBuffer backBuffer_;
    DepthBuffer depthBuffer_;
    InputManager input_;
    ResourceManager resources_;
    StateStack states_;
    float frameSeconds_ = 0.f;
};

} // namespace d25
