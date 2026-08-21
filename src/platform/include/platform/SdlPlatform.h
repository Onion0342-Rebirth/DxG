#pragma once
#include "platform/Platform.h"

// 前置声明，避免把 SDL 头暴露给所有包含者。
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace d25 {

// SDL2 平台实现。编译此翻译单元需要 SDL2 开发头文件；
// 不含 SDL 的构建可只链接其余模块（不包含此文件）。
class SdlPlatform : public Platform {
public:
    SdlPlatform(int width, int height);
    ~SdlPlatform() override;

    // 创建窗口/渲染器/纹理。失败返回 false（getError 可查看原因）。
    bool init();
    void shutdown();

    int width() const override { return width_; }
    int height() const override { return height_; }

    bool pollEvent(InputEvent& out) override;
    void present(const PixelBuffer& frame) override;

    const char* getError() const;

private:
    int width_;
    int height_;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
};

} // namespace d25
