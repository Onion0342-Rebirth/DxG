#pragma once
#include "input/InputEvent.h"

namespace d25 {

class PixelBuffer;

// 平台抽象：窗口创建、事件轮询、把帧缓冲上屏。
// 唯一允许接触具体平台 API（SDL2 等）的边界；渲染/玩法只依赖此接口。
class Platform {
public:
    virtual ~Platform() = default;

    virtual int width() const = 0;
    virtual int height() const = 0;

    // 取下一个事件；无事件返回 false（out 不变）。
    virtual bool pollEvent(InputEvent& out) = 0;

    // 把一帧像素缓冲呈现到窗口。
    virtual void present(const PixelBuffer& frame) = 0;
};

} // namespace d25
