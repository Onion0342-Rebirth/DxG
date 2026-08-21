#pragma once
#include "core/Color.h"
#include <string>

namespace d25 {

class PixelBuffer;
class UIRenderer;
class Character;

// 抬头显示：操作提示、FPS、坐标、临时提示（toast）。
// 不拥有 UIRenderer/Font，由调用方保证其生命周期覆盖 HUD。
class HUD {
public:
    explicit HUD(UIRenderer& ui) : ui_(&ui) {}

    void update(float dt);
    void showToast(const std::string& message, float duration = 2.5f);

    void render(PixelBuffer& buf, const Character* player, float fps) const;

private:
    UIRenderer* ui_;
    std::string toast_;
    float toastTime_ = 0.f;
};

} // namespace d25
