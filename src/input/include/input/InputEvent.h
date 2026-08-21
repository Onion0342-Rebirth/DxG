#pragma once
#include "input/Key.h"
#include <cstdint>

namespace d25 {

// 平台层产出的输入事件，GameApp 转发给 InputManager。
struct InputEvent {
    enum class Type : uint8_t {
        None = 0,     // 空事件（忽略）
        Quit,         // 窗口关闭 / 系统退出请求
        KeyDown,
        KeyUp,
    };

    Type type = Type::None;
    Key key = Key::None;
};

} // namespace d25
