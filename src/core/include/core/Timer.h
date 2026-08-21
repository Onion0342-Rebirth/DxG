#pragma once

namespace d25 {

// 固定步长计时器：累积真实时间，每次 step() 消耗一个固定步长。
// 主循环用法：while (timer.step()) 固定步长逻辑；每帧渲染用 frameDt() 做插值。
class Timer {
public:
    explicit Timer(float fixedDt = 1.0f / 60.0f);

    float fixedDt() const { return fixedDt_; }

    // 上一帧实际耗时（秒），用于渲染插值/UI 动画。
    float frameDt() const { return frameDt_; }

    // 若已累积满一个步长则消耗之并返回 true，否则返回 false。
    bool step();

private:
    static double nowSec();

    float fixedDt_;
    double last_;
    double accumulator_ = 0.0;
    float frameDt_ = 0.0f;
};

} // namespace d25
