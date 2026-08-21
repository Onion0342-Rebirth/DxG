#pragma once
#include "anim/AnimationClip.h"
#include <cmath>

namespace d25 {

// 动画播放器：内部计时，输出当前帧的绝对帧索引。
// 用法：play(clip) 切换剪辑（相同剪辑不重置），update(dt) 推进，frameIndex() 取帧。
class AnimationPlayer {
public:
    void play(const AnimationClip& clip) {
        if (clip_ == &clip) return;   // 同一剪辑重播不重置计时
        clip_ = &clip;
        time_ = 0.0f;
    }

    void update(float dt) {
        if (!clip_ || dt <= 0.f) return;
        time_ += dt;
        if (clip_->loop && clip_->duration() > 0.f) {
            time_ = std::fmod(time_, clip_->duration());
        }
    }

    // 当前帧的绝对帧索引（未设置剪辑时返回 0）。
    int frameIndex() const {
        if (!clip_) return 0;
        int f = clip_->firstFrame + int(time_ * clip_->fps);
        if (!clip_->loop) {
            const int last = clip_->firstFrame + clip_->frameCount - 1;
            f = f < last ? f : last;
        }
        return f;
    }

    bool finished() const {
        return clip_ && !clip_->loop && time_ >= clip_->duration();
    }

    const AnimationClip* clip() const { return clip_; }

private:
    const AnimationClip* clip_ = nullptr;
    float time_ = 0.0f;
};

} // namespace d25
