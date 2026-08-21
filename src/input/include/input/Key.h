#pragma once
#include <cstdint>

namespace d25 {

// 抽象按键：与具体平台（SDL 扫描码等）解耦。
// 平台层负责把平台键映射到这里的 Key。
enum class Key : uint8_t {
    None = 0,
    W, A, S, D,
    X, Z, Q,
    Up, Down, Left, Right,
    Space, Enter, Escape, Backspace,
    Count
};

} // namespace d25
