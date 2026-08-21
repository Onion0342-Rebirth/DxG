#pragma once
#include <cstdint>

namespace d25 {

// 确定性 LCG 随机数：相同种子永远产生相同序列。
// 用途：程序化纹理 / 程序化场景，保证"同一帧内容可复现"（也便于调试）。
class Rng {
public:
    explicit Rng(uint32_t seed) : state_(seed ? seed : 0x853c49e2u) {}

    // 下一个 32 位随机数。
    uint32_t next() {
        state_ = state_ * 1664525u + 1013904223u;
        return state_;
    }

    // [0, 1) 浮点。
    float nextFloat() { return (next() >> 8) * (1.0f / 16777216.0f); }

    // [0, n) 整数。
    uint32_t nextInt(uint32_t n) { return n > 0 ? next() % n : 0u; }

    uint32_t state() const { return state_; }

private:
    uint32_t state_;
};

} // namespace d25
