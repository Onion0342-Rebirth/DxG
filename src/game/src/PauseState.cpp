#include "game/PauseState.h"
#include "game/GameApp.h"
#include "ui/UIRenderer.h"
#include "core/PixelBuffer.h"
#include "input/InputManager.h"
#include "input/MouseButton.h"
#include "res/ResourceManager.h"
#include <algorithm>

namespace d25 {

namespace {

// 布局参数（像素）。
constexpr int kPanelPadX = 20;    // 面板左右内边距
constexpr int kPanelPadTop = 14;  // 面板顶部到标题的距离
constexpr int kPanelPadBottom = 16; // 面板底部内边距
constexpr int kButtonPadX = 16;   // 按钮文字左右内边距
constexpr int kButtonPadY = 6;    // 按钮文字上下内边距
constexpr int kButtonGap = 10;    // 标题与按钮、按钮之间的间距
constexpr int kSliderGap = 12;    // 退出按钮与滑块组之间的间距
constexpr int kLabelGap = 4;      // 滑块标签与滑道之间的间距
constexpr int kTrackH = 6;        // 滑道高度
constexpr int kKnobW = 8;         // 滑块旋钮宽度
constexpr int kKnobExtra = 3;     // 旋钮在滑道上下各伸出的高度

// 默认视野高度（无共享配置时使用，与 FollowCamConfig::height 默认一致）。
constexpr float kDefaultViewHeight = 9.0f;

// 点是否在矩形内（含边界）。
bool pointInRect(const PauseMenuRect& r, int x, int y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

// 当前视野配置：优先用共享配置，否则用一份默认配置（测试/无头）。
FollowCamConfig& viewConfig(FollowCamConfig* cfg) {
    static FollowCamConfig sDefault;
    return cfg ? *cfg : sDefault;
}

} // namespace

PauseState::PauseState(GameApp& app, FollowCamConfig* camCfg)
    : GameState(app), app_(app), camCfg_(camCfg) {}

void PauseState::onEnter() {}

PauseMenuLayout PauseState::computeLayout(const UIRenderer& ui, int screenW, int screenH) {
    const int lineH = ui.textHeight();

    // 按钮高度：文字行高 + 上下内边距。
    const int buttonH = lineH + kButtonPadY * 2;
    // 按钮宽度：两个按钮文案取宽者 + 左右内边距（两按钮等宽）。
    const int buttonTextW = std::max(ui.textWidth(kResumeText), ui.textWidth(kQuitText));
    const int buttonW = buttonTextW + kButtonPadX * 2;

    // 滑块组高度：VIEW 标签行高 + 标签与滑道间距 + 滑道高度。
    const int sliderGroupH = lineH + kLabelGap + kTrackH;

    const int panelW = buttonW + kPanelPadX * 2;
    const int panelH = kPanelPadTop + lineH + kButtonGap
                     + buttonH * 2 + kButtonGap
                     + kSliderGap + sliderGroupH + kPanelPadBottom;

    const int px = (screenW - panelW) / 2;
    const int py = (screenH - panelH) / 2;

    PauseMenuLayout layout;
    layout.panel = {px, py, panelW, panelH};

    const int bx = px + (panelW - buttonW) / 2;
    int by = py + kPanelPadTop + lineH + kButtonGap;
    layout.resumeButton = {bx, by, buttonW, buttonH};
    by += buttonH + kButtonGap;
    layout.quitButton = {bx, by, buttonW, buttonH};

    // 滑块滑道：与按钮等宽、水平居中，位于退出按钮下方。
    const int trackY = by + buttonH + kSliderGap + lineH + kLabelGap;
    layout.sliderTrack = {bx, trackY, buttonW, kTrackH};
    return layout;
}

int PauseState::hitTest(const PauseMenuLayout& layout, int x, int y) {
    if (pointInRect(layout.resumeButton, x, y)) return 1;
    if (pointInRect(layout.quitButton, x, y)) return 2;
    if (pointInRect(layout.sliderTrack, x, y)) return 3;
    return 0;
}

float PauseState::sliderValueAtX(const PauseMenuLayout& layout, int mouseX,
                                 float vMin, float vMax) {
    if (layout.sliderTrack.w <= 0) return vMin;
    const float t = float(mouseX - layout.sliderTrack.x) / float(layout.sliderTrack.w);
    const float tc = std::max(0.f, std::min(1.f, t));
    return vMin + (vMax - vMin) * tc;
}

int PauseState::sliderKnobX(const PauseMenuLayout& layout, float value,
                            float vMin, float vMax) {
    if (vMax <= vMin) return layout.sliderTrack.x;
    const float t = (value - vMin) / (vMax - vMin);
    const float tc = std::max(0.f, std::min(1.f, t));
    return layout.sliderTrack.x + int(tc * float(layout.sliderTrack.w));
}

void PauseState::update(float dt) {
    (void)dt;
    auto& in = app_.input();
    FollowCamConfig& cfg = viewConfig(camCfg_);

    // 键盘操作（保持不变）：Pause/Confirm/Cancel 恢复，Quit 退出。
    if (in.pressed(Action::Pause) ||
        in.pressed(Action::Confirm) ||
        in.pressed(Action::Cancel)) {
        app_.states().pop();
        return;
    }
    if (in.pressed(Action::Quit)) {
        app_.requestQuit();
        return;
    }

    const PauseMenuLayout layout = computeLayout(
        UIRenderer(ResourceManager::fallbackFont(), 2),
        app_.backBuffer().width(), app_.backBuffer().height());

    // 鼠标左键抬起：结束滑块拖动。
    if (in.mouseReleased(MouseButton::Left)) {
        draggingSlider_ = false;
    }

    // 鼠标左键按下：命中按钮执行按钮；命中滑道开始拖动并立即设值。
    if (in.mousePressed(MouseButton::Left)) {
        switch (hitTest(layout, in.mouseX(), in.mouseY())) {
            case 1: app_.states().pop(); return;
            case 2: app_.requestQuit(); return;
            case 3:
                draggingSlider_ = true;
                cfg.height = sliderValueAtX(layout, in.mouseX(),
                                            cfg.viewHeightMin, cfg.viewHeightMax);
                break;
            default: break;
        }
    }

    // 拖动中：按当前鼠标 x 连续更新视野高度。
    if (draggingSlider_ && in.mouseDown(MouseButton::Left)) {
        cfg.height = sliderValueAtX(layout, in.mouseX(),
                                    cfg.viewHeightMin, cfg.viewHeightMax);
    }
}

void PauseState::render() {
    // 不重画下层（画面保留），只叠加暂停面板。
    UIRenderer ui(ResourceManager::fallbackFont(), 2);
    auto& buf = app_.backBuffer();
    const PauseMenuLayout layout = computeLayout(ui, buf.width(), buf.height());
    const FollowCamConfig& cfg = viewConfig(camCfg_);
    drawOverlay(ui, buf, layout, app_.input().mouseX(), app_.input().mouseY(), cfg.height);
}

void PauseState::drawOverlay(UIRenderer& ui, PixelBuffer& buf,
                             const PauseMenuLayout& layout, int mouseX, int mouseY,
                             float viewValue) {
    const int w = buf.width();
    const int h = buf.height();

    // 半透明遮罩（逐像素 alpha 混合）。
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const Color c = buf.get(x, y);
            buf.set(x, y, Color::lerp(c, {0, 0, 0, 255}, 0.55f));
        }
    }

    const PauseMenuRect& p = layout.panel;
    ui.drawPanel(buf, p.x, p.y, p.w, p.h, {20, 24, 40, 235}, {90, 110, 150, 255});

    // 标题。
    const int tx = p.x + (p.w - ui.textWidth(kTitleText)) / 2;
    ui.drawText(buf, tx, p.y + kPanelPadTop, kTitleText, {240, 240, 240, 255});

    // 按钮：普通/悬停两种底色，文字居中。
    const Color buttonFill{40, 48, 76, 255};
    const Color buttonHoverFill{70, 90, 140, 255};
    const Color buttonBorder{120, 140, 190, 255};
    const Color buttonText{220, 228, 245, 255};

    const auto drawButton = [&](const PauseMenuRect& b, const char* text) {
        const bool hover = mouseX >= 0 && pointInRect(b, mouseX, mouseY);
        ui.drawPanel(buf, b.x, b.y, b.w, b.h,
                     hover ? buttonHoverFill : buttonFill, buttonBorder);
        const int textX = b.x + (b.w - ui.textWidth(text)) / 2;
        const int textY = b.y + kButtonPadY;
        ui.drawText(buf, textX, textY, text, buttonText);
    };

    drawButton(layout.resumeButton, kResumeText);
    drawButton(layout.quitButton, kQuitText);

    // 视野（VIEW）滑块：标签居中 + 滑道 + 旋钮。
    const FollowCamConfig defaults; // 仅取默认 min/max（绘制端）。
    const int labelX = layout.sliderTrack.x
                     + (layout.sliderTrack.w - ui.textWidth(kViewText)) / 2;
    const int labelY = layout.sliderTrack.y - kLabelGap - ui.textHeight();
    ui.drawText(buf, labelX, labelY, kViewText, {200, 210, 230, 255});

    const PauseMenuRect& tr = layout.sliderTrack;
    ui.drawRect(buf, tr.x, tr.y, tr.w, tr.h, {90, 100, 130, 255}); // 滑道
    const int knobCx = sliderKnobX(layout, viewValue,
                                   defaults.viewHeightMin, defaults.viewHeightMax);
    const int knobX = std::max(tr.x, std::min(tr.x + tr.w - kKnobW, knobCx - kKnobW / 2));
    ui.drawPanel(buf, knobX, tr.y - kKnobExtra, kKnobW, tr.h + kKnobExtra * 2,
                 {200, 210, 235, 255}, {240, 245, 255, 255}); // 旋钮
}

} // namespace d25
