#pragma once
#include <cstdint>

namespace d25 {

// 玩法动作：与具体按键解耦。按键 -> 动作的映射由 InputManager 配置。
// 未来加新动作（跳、菜单确认、翻滚……）在此扩展。
enum class Action : uint8_t {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Interact,
    Confirm,
    Cancel,
    Pause,
    Quit,
    Count
};

} // namespace d25
