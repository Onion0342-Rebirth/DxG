#include "world/Character.h"
#include "world/TileMap.h"
#include <algorithm>
#include <cmath>

namespace d25 {

Character::Character(const Vec2f& start) : pos_(start) {}

void Character::setWishDir(const Vec2f& d) {
    // 归一化，避免对角线加速。
    const float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len > 1e-6f) {
        wish_.x = d.x / len;
        wish_.y = d.y / len;
    } else {
        wish_ = {0.f, 0.f};
    }
}

void Character::setAnimClips(const AnimationClip* const idle[4], const AnimationClip* const walk[4]) {
    for (int i = 0; i < 4; ++i) {
        idle_[i] = idle[i];
        walk_[i] = walk[i];
    }
}

// 轴分离移动：先 X 再 Z，撞墙时沿另一轴滑动。
// blockedAt 是圆-瓦片判定：在角色半径范围内采样，出界/阻挡/水都视为阻挡。
bool Character::blockedAt(float x, float z, const TileMap& map) const {
    if (!map.inBounds(x, z)) return true;
    const float r = radius_;
    const float samples[3] = {-r, 0.f, r};
    for (float ox : samples) {
        for (float oz : samples) {
            if (!map.isWalkable(x + ox, z + oz)) return true;
        }
    }
    return false;
}

void Character::update(float dt, const TileMap& map) {
    moving_ = wish_.x != 0.f || wish_.y != 0.f;

    if (moving_) {
        // 按主导轴更新朝向（避免抖动）。
        if (std::abs(wish_.x) >= std::abs(wish_.y)) {
            facing_ = wish_.x > 0.f ? Direction::East : Direction::West;
        } else {
            facing_ = wish_.y > 0.f ? Direction::South : Direction::North;
        }

        // 轴分离：X
        const float nx = pos_.x + wish_.x * speed_ * dt;
        if (!blockedAt(nx, pos_.y, map)) pos_.x = nx;
        // 轴分离：Z
        const float ny = pos_.y + wish_.y * speed_ * dt;
        if (!blockedAt(pos_.x, ny, map)) pos_.y = ny;

        // 活动范围夹取（固定机位时用）。
        if (hasBounds_) {
            pos_.x = std::clamp(pos_.x, boundsMin_.x, boundsMax_.x);
            pos_.y = std::clamp(pos_.y, boundsMin_.y, boundsMax_.y);
        }
    }

    updateFacingAndAnim(dt);
}

void Character::updateFacingAndAnim(float dt) {
    // tag 相同不重置动画（走路不抖）；tag 变化切剪辑并 play 重置。
    const int tag = (moving_ ? 4 : 0) + int(facing_);
    if (tag != animTag_) {
        animTag_ = tag;
        const AnimationClip* clip = (moving_ ? walk_ : idle_)[int(facing_)];
        if (clip) anim_.play(*clip);
    }
    anim_.update(moving_ ? dt : 0.0f); // idle 暂不推进（可改为呼吸动画）
}

} // namespace d25
