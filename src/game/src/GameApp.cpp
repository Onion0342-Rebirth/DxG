#include "game/GameApp.h"
#include "game/GameplayState.h"
#include "platform/Platform.h"
#include <memory>

namespace d25 {

GameApp::GameApp(Platform& platform) : platform_(platform) {}
GameApp::~GameApp() = default;

void GameApp::onInit() {
    backBuffer_.resize(platform_.width(), platform_.height());
    depthBuffer_.resize(platform_.width(), platform_.height());
    input_.bindDefaults();

    // 框架默认进入基础玩法状态；正式游戏在此替换为你的启动菜单/初始场景。
    states_.push(std::make_unique<GameplayState>(*this));
}

void GameApp::onPreFrame(float dt) {
    frameSeconds_ = dt;
    input_.beginFrame();
    InputEvent ev;
    while (platform_.pollEvent(ev)) {
        if (ev.type == InputEvent::Type::Quit) {
            requestQuit();
            continue;
        }
        if (ev.type == InputEvent::Type::KeyDown && ev.key == Key::Q) {
            // 全局 Q 退出（暂停状态也消费了 Quit 动作，这里是全局兜底）。
        }
        input_.handleEvent(ev);
    }
    input_.endPoll();
}

void GameApp::onUpdate(float dt) {
    if (states_.top()) states_.top()->update(dt);
}

void GameApp::onRender() {
    if (states_.top()) states_.top()->render();
    platform_.present(backBuffer_);
}

void GameApp::onShutdown() {
    states_.clear();
}

} // namespace d25
