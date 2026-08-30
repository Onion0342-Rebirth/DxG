#pragma once
#include "core/Vec.h"
#include "world/character/Direction.h"
#include "anim/AnimationClip.h"
#include "anim/AnimationPlayer.h"

namespace d25 {

class TileMap;

// 2D 可移动角色（玩家/NPC/怪物的共同能力基类）：XZ 平面移动、8 方向输入归一化、
// 圆-瓦片碰撞（轴分离、撞墙滑动）。
// 动画按 8 方向 x (idle/walk) 提供剪辑，由资源层注入；切剪辑用 tag=(moving?8:0)+(int)facing 检测。
// 玩家特化见 world/player/Player.h；NPC、怪物可直接使用本类或派生。
class Character {
public:
    explicit Character(const Vec2f& start);

    const Vec2f& pos() const { return pos_; }
    void setPos(const Vec2f& p) { pos_ = p; }

    float speed() const { return speed_; }
    void setSpeed(float s) { speed_ = s; }

    float radius() const { return radius_; }
    void setRadius(float r) { radius_ = r; }

    // 活动范围（固定机位时把角色锁在视野内）。
    void setPlayBounds(const Vec2f& min, const Vec2f& max) {
        hasBounds_ = true; boundsMin_ = min; boundsMax_ = max;
    }
    void clearPlayBounds() { hasBounds_ = false; }

    // 注入 8 方向 idle / walk 动画剪辑（下标 = (int)Direction）。nullptr 允许（角色保持第 0 帧）。
    // idle 与 walk 均按时间循环推进，节奏由各自剪辑 fps 决定（idle 应比 walk 更慢）。
    void setAnimClips(const AnimationClip* const idle[8], const AnimationClip* const walk[8]);

    // 玩家/AI 输入的意愿方向（任意长度，内部归一化）。
    void setWishDir(const Vec2f& d);

    bool moving() const { return moving_; }
    Direction facing() const { return facing_; }
    int animFrame() const { return anim_.frameIndex(); }

    void update(float dt, const TileMap& map);

private:
    bool blockedAt(float x, float z, const TileMap& map) const;
    static Direction facingFromWish(const Vec2f& w);
    void updateFacingAndAnim(float dt);

    Vec2f pos_;
    Vec2f wish_{0.f, 0.f};
    float speed_ = 4.0f;
    float radius_ = 0.28f;

    bool hasBounds_ = false;
    Vec2f boundsMin_{0.f, 0.f};
    Vec2f boundsMax_{0.f, 0.f};

    bool moving_ = false;
    Direction facing_ = Direction::South;

    static constexpr int kDirCount = int(Direction::Count); // 8
    const AnimationClip* idle_[kDirCount] = {};
    const AnimationClip* walk_[kDirCount] = {};
    int animTag_ = -1;
    AnimationPlayer anim_;
};

} // namespace d25
