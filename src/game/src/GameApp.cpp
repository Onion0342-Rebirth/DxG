#include "game/GameApp.h"
#include "game/GameplayState.h"
#include "platform/Platform.h"
#include <cstdio>
#include <memory>
#include <string>
#include <sys/stat.h>

namespace d25 {

namespace {

// 判断某个 assets 目录是否可用（其下含 robot 角色素材）。
bool assetsDirValid(const std::string& dir) {
    struct stat st;
    return stat((dir + "/robot/stand/robot-stand-down.png").c_str(), &st) == 0;
}

// 定位 assets 目录：兼容从仓库根、DxG/、DxG/build/ 等不同工作目录启动。
// 找不到时返回空串（调用方走纯色兜底，不崩溃）。
std::string locateAssetsDir() {
    const char* candidates[] = {
        "assets",          // 从 DxG/ 启动
        "../assets",       // 从 DxG/build/ 启动
        "../../assets",    // 更深的构建子目录
        "DxG/assets",      // 从仓库根启动
        "./DxG/assets",
    };
    for (const char* c : candidates) {
        if (assetsDirValid(c)) return c;
    }
    return {};
}

} // namespace

GameApp::GameApp(Platform& platform) : platform_(platform) {}
GameApp::~GameApp() = default;

void GameApp::onInit() {
    backBuffer_.resize(platform_.width(), platform_.height());
    depthBuffer_.resize(platform_.width(), platform_.height());
    input_.bindDefaults();

    // 加载角色素材（PNG 精灵表 + 动画剪辑）；失败仅警告并继续走纯色兜底。
    const std::string assetsDir = locateAssetsDir();
    if (!assetsDir.empty()) {
        std::string err;
        if (!resources_.loadPlayerRobot(assetsDir, &err)) {
            std::fprintf(stderr, "[资源] 角色素材加载失败，使用纯色兜底: %s\n", err.c_str());
        }
    } else {
        std::fprintf(stderr, "[资源] 未找到 assets 目录，使用纯色兜底。\n");
    }

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
