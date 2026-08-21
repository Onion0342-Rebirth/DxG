#pragma once
#include <cstdint>

namespace d25 {

// 平面朝向（XZ 平面，正 Y 向上）。固定 4 方向，便于 2D 精灵按朝向索引。
enum class Direction : uint8_t {
    North = 0, // 朝向 -Z
    South = 1, // 朝向 +Z
    East = 2,  // 朝向 +X
    West = 3,  // 朝向 -X
};

} // namespace d25
