#include "core/Game.h"
#include "core/Timer.h"

namespace d25 {

void Game::run() {
    onInit();

    Timer timer(fixedDt_);
    while (!quit_) {
        onPreFrame(timer.frameDt());

        // 固定步长逻辑：一次性追到最新状态。
        while (timer.step()) {
            onUpdate(timer.fixedDt());
        }

        onRender();
    }

    onShutdown();
}

} // namespace d25
