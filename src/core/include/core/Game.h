#pragma once

namespace d25 {

// 主循环骨架：固定步长逻辑 + 每帧渲染。具体游戏由子类挂接钩子实现。
//
// 调用顺序：
//   onInit() -> [ onPreFrame -> (onUpdate x N) -> onRender ]* -> onShutdown()
// 其中 N 由 Timer 决定（逻辑步长固定为 1/60s，渲染每帧一次）。
class Game {
public:
    virtual ~Game() = default;

    // 启动主循环，直到 requestQuit()。
    void run();

    void requestQuit() { quit_ = true; }
    bool quitRequested() const { return quit_; }

protected:
    // 启动时：创建窗口缓冲、加载资源、压入初始状态。
    virtual void onInit() {}
    // 每帧开头：轮询/分发输入事件（输入帧开始）。
    virtual void onPreFrame(float /*dt*/) {}
    // 固定步长逻辑（dt = fixedDt）。状态栈的 update 在这里驱动。
    virtual void onUpdate(float /*dt*/) {}
    // 每帧渲染（在固定步长之后，逻辑状态之间的插值在这里做）。
    virtual void onRender() {}
    // 退出时：释放资源、清空状态栈。
    virtual void onShutdown() {}

private:
    bool quit_ = false;
    float fixedDt_ = 1.0f / 60.0f;
};

} // namespace d25
