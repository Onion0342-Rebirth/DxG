#pragma once
#include "world/character/Character.h"

namespace d25 {

// 玩家角色：继承 Character 的移动/碰撞/动画能力。
// 玩家独有的数据与规则（未来：等级、装备、交互状态等）放在本类；
// NPC、怪物等非玩家角色直接使用 Character 或其派生类，由 World::characters() 持有。
class Player : public Character {
public:
    explicit Player(const Vec2f& start) : Character(start) {}
};

} // namespace d25
