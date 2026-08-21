#include "game/PauseState.h"
#include "game/GameApp.h"
#include "ui/UIRenderer.h"
#include "core/PixelBuffer.h"
#include <algorithm>

namespace d25 {

PauseState::PauseState(GameApp& app) : GameState(app), app_(app) {}

void PauseState::onEnter() {}

void PauseState::update(float dt) {
    (void)dt;
    auto& in = app_.input();
    if (in.pressed(Action::Pause) ||
        in.pressed(Action::Confirm) ||
        in.pressed(Action::Cancel)) {
        app_.states().pop();
        return;
    }
    if (in.pressed(Action::Quit)) {
        app_.requestQuit();
    }
}

void PauseState::render() {
    // 不重画下层（画面保留），只叠加暂停面板。
    UIRenderer ui(ResourceManager::fallbackFont(), 2);
    drawOverlay(ui, app_.backBuffer());
}

void PauseState::drawOverlay(UIRenderer& ui, PixelBuffer& buf) {
    const int w = buf.width();
    const int h = buf.height();

    // 半透明遮罩（逐像素 alpha 混合）。
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const Color c = buf.get(x, y);
            buf.set(x, y, Color::lerp(c, {0, 0, 0, 255}, 0.55f));
        }
    }

    const int pw = std::max(ui.textWidth("ESC / ENTER RESUME") + 40, 200);
    const int ph = ui.textHeight() * 3 + 40;
    const int px = (w - pw) / 2;
    const int py = (h - ph) / 2;

    ui.drawPanel(buf, px, py, pw, ph, {20, 24, 40, 235}, {90, 110, 150, 255});

    const char* title = "PAUSED";
    const int tx = px + (pw - ui.textWidth(title)) / 2;
    ui.drawText(buf, tx, py + 12, title, {240, 240, 240, 255});

    const char* resume = "ESC / ENTER RESUME";
    ui.drawText(buf, px + 16, py + 12 + ui.textHeight() + 8, resume, {200, 210, 230, 255});

    const char* quit = "Q QUIT";
    ui.drawText(buf, px + 16, py + 12 + (ui.textHeight() + 8) * 2, quit, {200, 210, 230, 255});
}

} // namespace d25
