#pragma once
#include "core/Color.h"
#include "game/GameState.h"
#include "render/FollowCam.h"

namespace d25 {

class GameApp;
class UIRenderer;
class PixelBuffer;

// 菜单矩形（帧缓冲像素坐标）。
struct PauseMenuRect {
    int x = 0, y = 0, w = 0, h = 0;
};

// 暂停菜单布局：面板、两个按钮与视野滑块滑道的矩形。
struct PauseMenuLayout {
    PauseMenuRect panel;
    PauseMenuRect resumeButton;  // 继续
    PauseMenuRect quitButton;    // 退出
    PauseMenuRect sliderTrack;   // 视野（VIEW）滑块滑道
};

// 暂停状态：画面定格（不 update 下层），居中显示暂停面板。
// 键盘：Pause/Confirm/Cancel 恢复，Quit 退出；
// 鼠标：悬停按钮高亮，左键点击 RESUME 恢复、点击 QUIT 退出；
//       按住/点击 VIEW 滑块滑道左右拖动可调整相机高度（视野远近），恢复后生效。
class PauseState : public GameState {
public:
    // camCfg 为可选的跟随相机配置（来自 GameplayState）；为空时使用默认配置（测试/无头用）。
    explicit PauseState(GameApp& app, FollowCamConfig* camCfg = nullptr);

    void onEnter() override;
    void update(float dt) override;
    void render() override;

    // 文案常量（布局/绘制/测试共用；内置字体仅支持 ASCII，故用英文）。
    static constexpr const char* kTitleText = "PAUSED";
    static constexpr const char* kResumeText = "RESUME";
    static constexpr const char* kQuitText = "QUIT";
    static constexpr const char* kViewText = "VIEW"; // 视野滑块标签

    // 依据屏幕尺寸与 UI 度量计算菜单布局（纯函数，供测试）。
    static PauseMenuLayout computeLayout(const UIRenderer& ui, int screenW, int screenH);

    // 命中测试：返回 0=未命中，1=继续(RESUME)，2=退出(QUIT)，3=视野滑块滑道。
    static int hitTest(const PauseMenuLayout& layout, int x, int y);

    // 滑块滑道内某像素 x 对应的视野值（线性映射到 [vMin,vMax]，越界 clamp）。
    static float sliderValueAtX(const PauseMenuLayout& layout, int mouseX,
                                float vMin, float vMax);

    // 视野值对应的滑块旋钮中心 x（与 sliderValueAtX 互逆）。
    static int sliderKnobX(const PauseMenuLayout& layout, float value,
                           float vMin, float vMax);

    // 供测试/无头复用：在给定缓冲上按布局画暂停面板。
    // mouseX/mouseY 为鼠标坐标（<0 表示无鼠标，不做悬停高亮）。
    // viewValue 为当前视野（相机高度），用于绘制滑块旋钮位置。
    static void drawOverlay(UIRenderer& ui, PixelBuffer& buf,
                            const PauseMenuLayout& layout, int mouseX, int mouseY,
                            float viewValue);

private:
    GameApp& app_;
    FollowCamConfig* camCfg_;   // 共享的跟随相机配置（可能为 nullptr）
    bool draggingSlider_ = false; // 鼠标左键是否正在拖动视野滑块
};

} // namespace d25
