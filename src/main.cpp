// DxG 入口：创建 SDL 平台与 GameApp，进入主循环。
// 注意：编译本文件需要 SDL2 开发头文件；没有 SDL2 时，可只编译其余模块为库。

#include "platform/SdlPlatform.h"
#include "game/GameApp.h"

#include <cstdio>

int main(int, char**) {
    d25::SdlPlatform platform(960, 600);
    if (!platform.init()) {
        std::fprintf(stderr, "SDL init failed: %s\n", platform.getError());
        return 1;
    }

    d25::GameApp app(platform);
    app.run();
    return 0;
}
