#pragma once
#include "render/Sprite.h"
#include <vector>

namespace d25 {

// 精灵表：一组等尺寸帧。AnimationPlayer 输出的绝对帧索引在此取图。
class SpriteSheet {
public:
    void add(const Sprite& s) { frames_.push_back(s); }
    void add(Sprite&& s) { frames_.push_back(std::move(s)); }

    const Sprite& frame(int i) const { return frames_[size_t(i)]; }
    Sprite& frame(int i) { return frames_[size_t(i)]; }

    int frameCount() const { return int(frames_.size()); }
    bool empty() const { return frames_.empty(); }

    void clear() { frames_.clear(); }

private:
    std::vector<Sprite> frames_;
};

} // namespace d25
