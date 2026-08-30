#pragma once
#include "input/Key.h"
#include "input/MouseButton.h"
#include <cstdint>

namespace d25 {

// 平台层产出的输入事件，GameApp 转发给 InputManager。
struct InputEvent {
    enum class Type : uint8_t {
        None = 0,     // 空事件（忽略）
        Quit,         // 窗口关闭 / 系统退出请求
        KeyDown,
        KeyUp,
        MouseMove,    // 鼠标移动（坐标在 mouseX/mouseY）
        MouseDown,    // 鼠标按键按下（按键在 button，坐标在 mouseX/mouseY）
        MouseUp,      // 鼠标按键抬起
    };

    Type type = Type::None;
    Key key = Key::None;

    // 鼠标事件专用字段；键盘事件不使用（保持默认值）。
    MouseButton button = MouseButton::None;
    int mouseX = 0;
    int mouseY = 0;
};

} // namespace d25
