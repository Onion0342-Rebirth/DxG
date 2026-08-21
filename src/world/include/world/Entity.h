#pragma once
#include "core/Vec.h"
#include <cstdint>

namespace d25 {

// 静态物件的逻辑身份：只描述"是什么、在哪、多大"，渲染层决定怎么画。
enum class EntityKind : uint8_t {
    Tree = 0,
    Rock,
    House,
    Crate,
    Count
};

struct Entity {
    EntityKind kind = EntityKind::Tree;
    Vec2f pos{0.f, 0.f}; // XZ 平面世界坐标
    float scale = 1.0f;
};

} // namespace d25
