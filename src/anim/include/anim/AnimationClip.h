#pragma once

namespace d25 {

// 动画剪辑：只描述"从哪一帧开始、共几帧、多快、是否循环"。
// 帧索引是精灵表内的【绝对索引】；本模块不知道精灵长什么样，与 render 解耦。
struct AnimationClip {
    int firstFrame = 0;
    int frameCount = 1;
    float fps = 8.0f;
    bool loop = true;

    float duration() const {
        return frameCount > 0 ? float(frameCount) / fps : 0.0f;
    }
};

} // namespace d25
