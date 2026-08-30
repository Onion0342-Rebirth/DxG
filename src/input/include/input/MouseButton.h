#pragma once
#include <cstdint>

namespace d25 {

// 抽象鼠标按键：与具体平台（SDL 按键码等）解耦。
// 平台层负责把平台按键映射到这里的 MouseButton。
enum class MouseButton : uint8_t {
    None = 0,
    Left,
    Right,
    Middle,
    Count
};

} // namespace d25
