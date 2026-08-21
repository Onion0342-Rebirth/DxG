#include "ui/HUD.h"
#include "ui/UIRenderer.h"
#include "core/PixelBuffer.h"
#include "world/Character.h"
#include <cstdio>

namespace d25 {

void HUD::update(float dt) {
    if (toastTime_ > 0.f) {
        toastTime_ -= dt;
        if (toastTime_ <= 0.f) {
            toastTime_ = 0.f;
            toast_.clear();
        }
    }
}

void HUD::showToast(const std::string& message, float duration) {
    toast_ = message;
    toastTime_ = duration;
}

void HUD::render(PixelBuffer& buf, const Character* player, float fps) const {
    if (!ui_) return;

    const Color white{240, 240, 240, 255};
    const Color shadow{0, 0, 0, 200};

    // 左上：操作提示（简单双 pass 做阴影）。
    const char* hint = "WASD MOVE  |  ESC PAUSE";
    const int x = 8, y = 8;
    ui_->drawText(buf, x + 1, y + 1, hint, shadow);
    ui_->drawText(buf, x, y, hint, white);

    // 右上：FPS。
    char fpsBuf[32];
    std::snprintf(fpsBuf, sizeof(fpsBuf), "FPS %.0f", fps);
    const int fw = ui_->textWidth(fpsBuf);
    ui_->drawText(buf, buf.width() - fw - 8 + 1, y + 1, fpsBuf, shadow);
    ui_->drawText(buf, buf.width() - fw - 8, y, fpsBuf, white);

    // 左下：坐标。
    if (player) {
        char posBuf[48];
        std::snprintf(posBuf, sizeof(posBuf), "POS %.1f %.1f",
                      player->pos().x, player->pos().y);
        const int py = buf.height() - ui_->textHeight() - 8;
        ui_->drawText(buf, x + 1, py + 1, posBuf, shadow);
        ui_->drawText(buf, x, py, posBuf, white);
    }

    // 底部居中：toast。
    if (!toast_.empty() && toastTime_ > 0.f) {
        const int tw = ui_->textWidth(toast_);
        const int px = (buf.width() - tw) / 2;
        const int py = buf.height() - ui_->textHeight() - 32;
        ui_->drawText(buf, px + 1, py + 1, toast_, shadow);
        ui_->drawText(buf, px, py, toast_, white);
    }
}

} // namespace d25
