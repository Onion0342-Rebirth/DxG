#pragma once
#include "input/Key.h"
#include "input/InputAction.h"
#include "input/InputEvent.h"
#include <array>

namespace d25 {

// 输入管理器：把平台事件（KeyDown/KeyUp）汇成"本帧状态"，
// 再提供 down / pressed / released 三态查询（按玩法动作或按键）。
//
// 每帧调用约定（由 GameApp::onPreFrame 驱动）：
//   beginFrame() -> handleEvent(...) xN -> endPoll()
class InputManager {
public:
    static constexpr int kMaxKeysPerAction = 4;

    // 默认键位：WASD + 方向键移动；X/Space 交互；Enter/Space 确认；
    // Backspace 取消；Esc 暂停；Q 退出。
    void bindDefaults();

    // 绑定动作 -> 按键（可多键映射同一动作，达到上限后忽略）。
    void mapKey(Action action, Key key);

    void beginFrame();
    void handleEvent(const InputEvent& ev);
    void endPoll();

    bool down(Key key) const;
    bool pressed(Key key) const;
    bool released(Key key) const;

    bool down(Action action) const;
    bool pressed(Action action) const;
    bool released(Action action) const;

private:
    std::array<bool, size_t(Key::Count)> keysDown_{};
    std::array<bool, size_t(Key::Count)> keysPressed_{};
    std::array<bool, size_t(Key::Count)> keysReleased_{};

    std::array<std::array<Key, kMaxKeysPerAction>, size_t(Action::Count)> bindings_{};
    std::array<int, size_t(Action::Count)> bindingCount_{};
};

} // namespace d25
