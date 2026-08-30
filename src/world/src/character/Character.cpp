#include "world/character/Character.h"
#include "world/terrain/TileMap.h"
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

void Character::setAnimClips(const AnimationClip* const idle[8], const AnimationClip* const walk[8]) {
    for (int i = 0; i < kDirCount; ++i) {
        idle_[i] = idle[i];
        walk_[i] = walk[i];
    }
}

// 把归一化后的意愿方向（x=X，y=Z）量化到最近的 45° 八方向。
// 世界角度：atan2(z, x) 以 +X（东）为 0、顺时针（+Z=南）为正；
// 乘 4/π 后四舍五入得到 0..7 的八分区（0=东，顺时针），再映射到 Direction 枚举
// （枚举从北起顺时针：dirIdx = (octant + 2) % 8）。
Direction Character::facingFromWish(const Vec2f& w) {
    float oct = std::round(std::atan2(w.y, w.x) * 4.f / 3.14159265358979323846f);
    int idx = int(oct) % 8;
    if (idx < 0) idx += 8;
    return Direction((idx + 2) % 8);
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
        // 把移动方向量化到最近的 45° 八方向（纯轴方向与原四方向结果一致，
        // 等分量斜向落到 NE/SE/SW/NW），避免斜走时动画在两个轴向间抖动。
        facing_ = facingFromWish(wish_);

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
    const int tag = (moving_ ? 8 : 0) + int(facing_);
    if (tag != animTag_) {
        animTag_ = tag;
        const AnimationClip* clip = (moving_ ? walk_ : idle_)[int(facing_)];
        if (clip) anim_.play(*clip);
    }
    // idle 与 walk 都按真实时间推进；快慢由各自剪辑的 fps 决定
    // （walk 较快、idle 较慢循环，呈现待机呼吸感，而不是静止僵住）。
    anim_.update(dt);
}

} // namespace d25
