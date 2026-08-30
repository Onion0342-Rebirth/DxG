// 分类：game
// 暂停菜单测试（不依赖 SDL）：
//   1. computeLayout 布局合理性（按钮在面板内、不越界、两按钮不重叠）；
//   2. hitTest 命中判定（继续/退出按钮中心、面板外、按钮外）；
//   3. drawOverlay 冒烟测试（可绘制，悬停与非悬停按钮区域颜色不同）。
#include "framework/test_framework.h"
#include "game/PauseState.h"
#include "render/FollowCam.h"
#include "ui/UIRenderer.h"
#include "res/ResourceManager.h"
#include "core/PixelBuffer.h"
#include "core/Color.h"
#include <cmath>

namespace {

using namespace d25;

bool nearly(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) < eps; }

bool contains(const PauseMenuRect& outer, const PauseMenuRect& inner) {
    return inner.x >= outer.x &&
           inner.y >= outer.y &&
           inner.x + inner.w <= outer.x + outer.w &&
           inner.y + inner.h <= outer.y + outer.h;
}

bool overlaps(const PauseMenuRect& a, const PauseMenuRect& b) {
    return !(a.x + a.w <= b.x || b.x + b.w <= a.x ||
             a.y + a.h <= b.y || b.y + b.h <= a.y);
}

// 统计两个缓冲在指定矩形区域内"颜色明显不同"的像素数。
long countDiffInRect(const PixelBuffer& a, const PixelBuffer& b, const PauseMenuRect& r) {
    long diff = 0;
    for (int y = r.y; y < r.y + r.h; ++y) {
        for (int x = r.x; x < r.x + r.w; ++x) {
            const Color ca = a.get(x, y);
            const Color cb = b.get(x, y);
            const int dr = int(ca.r) - int(cb.r);
            const int dg = int(ca.g) - int(cb.g);
            const int db = int(ca.b) - int(cb.b);
            if (dr * dr + dg * dg + db * db > 25 * 25) ++diff;
        }
    }
    return diff;
}

} // namespace

void runPauseMenuTests() {
    using namespace d25test;

    constexpr int kScreenW = 320;
    constexpr int kScreenH = 240;
    UIRenderer ui(ResourceManager::fallbackFont(), 2);
    const PauseMenuLayout layout = PauseState::computeLayout(ui, kScreenW, kScreenH);

    beginCase("面板位于屏幕内且尺寸为正");
    CHECK(layout.panel.w > 0);
    CHECK(layout.panel.h > 0);
    CHECK(layout.panel.x >= 0);
    CHECK(layout.panel.y >= 0);
    CHECK(layout.panel.x + layout.panel.w <= kScreenW);
    CHECK(layout.panel.y + layout.panel.h <= kScreenH);

    beginCase("两个按钮都在面板内部");
    CHECK(contains(layout.panel, layout.resumeButton));
    CHECK(contains(layout.panel, layout.quitButton));

    beginCase("两个按钮等宽、不重叠，且继续在退出上方");
    CHECK(layout.resumeButton.w == layout.quitButton.w);
    CHECK(layout.resumeButton.h == layout.quitButton.h);
    CHECK(!overlaps(layout.resumeButton, layout.quitButton));
    CHECK(layout.resumeButton.y < layout.quitButton.y);

    // VIEW 滑块滑道布局。
    beginCase("滑块滑道在面板内、与按钮等宽、位于退出按钮下方且不重叠");
    CHECK(layout.sliderTrack.w > 0);
    CHECK(layout.sliderTrack.h > 0);
    CHECK(contains(layout.panel, layout.sliderTrack));
    CHECK(layout.sliderTrack.w == layout.resumeButton.w);
    CHECK(layout.sliderTrack.y > layout.quitButton.y);
    CHECK(!overlaps(layout.sliderTrack, layout.quitButton));
    CHECK(!overlaps(layout.sliderTrack, layout.resumeButton));

    // hitTest
    beginCase("命中继续按钮中心 -> 1");
    const int resumeCx = layout.resumeButton.x + layout.resumeButton.w / 2;
    const int resumeCy = layout.resumeButton.y + layout.resumeButton.h / 2;
    CHECK(PauseState::hitTest(layout, resumeCx, resumeCy) == 1);

    beginCase("命中退出按钮中心 -> 2");
    const int quitCx = layout.quitButton.x + layout.quitButton.w / 2;
    const int quitCy = layout.quitButton.y + layout.quitButton.h / 2;
    CHECK(PauseState::hitTest(layout, quitCx, quitCy) == 2);

    beginCase("点面板外（左上角空白）-> 0");
    CHECK(PauseState::hitTest(layout, 2, 2) == 0);

    beginCase("点标题区域/按钮外（面板顶部内边距）-> 0");
    CHECK(PauseState::hitTest(layout, layout.panel.x + 2, layout.panel.y + 2) == 0);

    beginCase("点按钮角点（含边界判定）命中对应按钮");
    CHECK(PauseState::hitTest(layout, layout.resumeButton.x, layout.resumeButton.y) == 1);
    CHECK(PauseState::hitTest(layout, layout.quitButton.x + layout.quitButton.w,
                             layout.quitButton.y + layout.quitButton.h) == 2);

    // 滑块命中。
    const int trackCx = layout.sliderTrack.x + layout.sliderTrack.w / 2;
    const int trackCy = layout.sliderTrack.y + layout.sliderTrack.h / 2;
    beginCase("命中滑块滑道 -> 3");
    CHECK(PauseState::hitTest(layout, trackCx, trackCy) == 3);
    beginCase("滑块滑道左端/右端角点也命中 3");
    CHECK(PauseState::hitTest(layout, layout.sliderTrack.x, trackCy) == 3);
    CHECK(PauseState::hitTest(layout, layout.sliderTrack.x + layout.sliderTrack.w, trackCy) == 3);

    // 滑块取值映射。
    const FollowCamConfig cfgDefaults;
    const float vMin = cfgDefaults.viewHeightMin;
    const float vMax = cfgDefaults.viewHeightMax;
    beginCase("sliderValueAtX：滑道左端=vMin、右端=vMax、中点=中值，越界 clamp");
    CHECK(nearly(PauseState::sliderValueAtX(layout, layout.sliderTrack.x, vMin, vMax), vMin));
    CHECK(nearly(PauseState::sliderValueAtX(layout, layout.sliderTrack.x + layout.sliderTrack.w, vMin, vMax), vMax));
    CHECK(nearly(PauseState::sliderValueAtX(layout, trackCx, vMin, vMax), (vMin + vMax) * 0.5f));
    CHECK(nearly(PauseState::sliderValueAtX(layout, layout.sliderTrack.x - 100, vMin, vMax), vMin));
    CHECK(nearly(PauseState::sliderValueAtX(layout, layout.sliderTrack.x + layout.sliderTrack.w + 100, vMin, vMax), vMax));

    beginCase("sliderKnobX 与 sliderValueAtX 互逆");
    const int knobAtMin = PauseState::sliderKnobX(layout, vMin, vMin, vMax);
    const int knobAtMax = PauseState::sliderKnobX(layout, vMax, vMin, vMax);
    const int knobAtMid = PauseState::sliderKnobX(layout, (vMin + vMax) * 0.5f, vMin, vMax);
    CHECK(knobAtMin == layout.sliderTrack.x);
    CHECK(knobAtMax >= layout.sliderTrack.x + layout.sliderTrack.w - 1);
    CHECK(nearly(float(knobAtMid), float(layout.sliderTrack.x + layout.sliderTrack.w / 2), 1.5f));
    // 由旋钮 x 反推值应回到原值。
    CHECK(nearly(PauseState::sliderValueAtX(layout, knobAtMid, vMin, vMax), (vMin + vMax) * 0.5f, 0.5f));

    // drawOverlay 冒烟测试：同一底色缓冲分别画"无鼠标"与"鼠标悬停在继续按钮上"。
    PixelBuffer plain(kScreenW, kScreenH);
    PixelBuffer hovered(kScreenW, kScreenH);
    for (int i = 0; i < kScreenW * kScreenH; ++i) {
        plain.data()[i] = Color{30, 30, 30, 255};
        hovered.data()[i] = Color{30, 30, 30, 255};
    }

    PauseState::drawOverlay(ui, plain, layout, -1, -1, vMin);
    PauseState::drawOverlay(ui, hovered, layout, resumeCx, resumeCy, vMin);

    beginCase("绘制后按钮区域相对纯遮罩底色有内容（按钮被画出）");
    // 取一个"无按钮、只有遮罩"的参照点：屏幕左上角（面板外）。
    const Color maskRef = plain.get(2, 2);
    const Color buttonPixel = plain.get(resumeCx, resumeCy);
    const int dr = int(maskRef.r) - int(buttonPixel.r);
    const int dg = int(maskRef.g) - int(buttonPixel.g);
    const int db = int(maskRef.b) - int(buttonPixel.b);
    CHECK(dr * dr + dg * dg + db * db > 25 * 25);

    beginCase("悬停时继续按钮区域与非悬停不同（高亮生效）");
    const long hoverDiff = countDiffInRect(plain, hovered, layout.resumeButton);
    std::printf("    [info] 继续按钮悬停差异像素 = %ld\n", hoverDiff);
    CHECK(hoverDiff > 20);

    beginCase("悬停在继续按钮时，退出按钮区域不变");
    const long quitDiff = countDiffInRect(plain, hovered, layout.quitButton);
    CHECK(quitDiff == 0);

    // 滑块旋钮绘制：不同视野值旋钮位置不同 -> 滑道区域像素应有差异。
    PixelBuffer lowView(kScreenW, kScreenH);
    PixelBuffer highView(kScreenW, kScreenH);
    for (int i = 0; i < kScreenW * kScreenH; ++i) {
        lowView.data()[i] = Color{30, 30, 30, 255};
        highView.data()[i] = Color{30, 30, 30, 255};
    }
    // 滑道上下各留若干像素，覆盖旋钮伸出滑道的部分。
    PauseMenuRect sliderZone = layout.sliderTrack;
    sliderZone.y -= 4;
    sliderZone.h += 8;
    PauseState::drawOverlay(ui, lowView, layout, -1, -1, vMin);
    PauseState::drawOverlay(ui, highView, layout, -1, -1, vMax);
    beginCase("不同视野值下滑块旋钮位置不同（滑道区域像素有差异）");
    const long sliderDiff = countDiffInRect(lowView, highView, sliderZone);
    std::printf("    [info] 滑块最小/最大视野差异像素 = %ld\n", sliderDiff);
    CHECK(sliderDiff > 5);
}
