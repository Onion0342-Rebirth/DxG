#pragma once
#include <cstdint>

namespace d25 {

// 平面朝向（XZ 平面，正 Y 向上）。固定 8 方向（每 45° 一个），便于 2D 精灵按朝向索引。
// 枚举顺序 = 从正北起顺时针，且枚举值即角色精灵表内"每方向帧数"的基址倍数，
// 资源层拼表/建剪辑时直接用 (int)Direction 作下标，勿随意重排。
enum class Direction : uint8_t {
    North = 0,     // 朝向 -Z
    NorthEast = 1, // 朝向 +X -Z
    East = 2,      // 朝向 +X
    SouthEast = 3, // 朝向 +X +Z
    South = 4,     // 朝向 +Z
    SouthWest = 5, // 朝向 -X +Z
    West = 6,      // 朝向 -X
    NorthWest = 7, // 朝向 -X -Z
    Count = 8,
};

} // namespace d25
