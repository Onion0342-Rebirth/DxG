// 分类：input
// 鼠标输入测试（不依赖 SDL）：直接向 InputManager 注入 InputEvent，
// 验证鼠标位置跟踪、按键三态（down/pressed/released）以及键盘/鼠标互不干扰。
#include "framework/test_framework.h"
#include "input/InputManager.h"
#include "input/InputEvent.h"
#include "input/MouseButton.h"
#include "input/Key.h"

namespace {

using namespace d25;

InputEvent mouseMove(int x, int y) {
    InputEvent ev;
    ev.type = InputEvent::Type::MouseMove;
    ev.mouseX = x;
    ev.mouseY = y;
    return ev;
}

InputEvent mouseButton(InputEvent::Type type, MouseButton btn, int x, int y) {
    InputEvent ev;
    ev.type = type;
    ev.button = btn;
    ev.mouseX = x;
    ev.mouseY = y;
    return ev;
}

InputEvent key(InputEvent::Type type, Key k) {
    InputEvent ev;
    ev.type = type;
    ev.key = k;
    return ev;
}

} // namespace

void runMouseInputTests() {
    using namespace d25test;

    InputManager in;

    beginCase("初始状态：尚无鼠标移动，坐标为 -1");
    CHECK(in.mouseX() == -1);
    CHECK(in.mouseY() == -1);
    CHECK(!in.mouseDown(MouseButton::Left));

    // 帧 1：移动 + 左键按下。
    in.beginFrame();
    in.handleEvent(mouseMove(100, 50));
    in.handleEvent(mouseButton(InputEvent::Type::MouseDown, MouseButton::Left, 100, 50));
    in.endPoll();

    beginCase("MouseMove 更新鼠标坐标");
    CHECK(in.mouseX() == 100);
    CHECK(in.mouseY() == 50);

    beginCase("MouseDown 当帧：pressed 与 down 均为真");
    CHECK(in.mousePressed(MouseButton::Left));
    CHECK(in.mouseDown(MouseButton::Left));
    CHECK(!in.mouseReleased(MouseButton::Left));

    beginCase("未按的右键三态均为假");
    CHECK(!in.mousePressed(MouseButton::Right));
    CHECK(!in.mouseDown(MouseButton::Right));

    // 帧 2：无事件。边沿应清空，按住状态保留。
    in.beginFrame();
    in.endPoll();
    beginCase("下一帧：pressed 清零，down 仍为真，位置保留");
    CHECK(!in.mousePressed(MouseButton::Left));
    CHECK(in.mouseDown(MouseButton::Left));
    CHECK(in.mouseX() == 100);
    CHECK(in.mouseY() == 50);

    // 帧 3：左键抬起。
    in.beginFrame();
    in.handleEvent(mouseButton(InputEvent::Type::MouseUp, MouseButton::Left, 110, 60));
    in.endPoll();
    beginCase("MouseUp 当帧：released 为真，down 变假，坐标更新");
    CHECK(in.mouseReleased(MouseButton::Left));
    CHECK(!in.mouseDown(MouseButton::Left));
    CHECK(!in.mousePressed(MouseButton::Left));
    CHECK(in.mouseX() == 110);
    CHECK(in.mouseY() == 60);

    // 帧 4：无事件。释放边沿清空。
    in.beginFrame();
    in.endPoll();
    beginCase("再下一帧：released 清零");
    CHECK(!in.mouseReleased(MouseButton::Left));

    // 键盘/鼠标互不干扰。
    in.beginFrame();
    in.handleEvent(key(InputEvent::Type::KeyDown, Key::W));
    in.handleEvent(mouseButton(InputEvent::Type::MouseDown, MouseButton::Right, 5, 6));
    in.endPoll();
    beginCase("同帧键盘与鼠标事件互不干扰");
    CHECK(in.down(Key::W));
    CHECK(in.mouseDown(MouseButton::Right));
    CHECK(!in.mouseDown(MouseButton::Left));
    // 键盘事件不应产生鼠标边沿，反之亦然。
    CHECK(!in.mousePressed(MouseButton::Left));
    CHECK(!in.mousePressed(MouseButton::Middle));

    in.beginFrame();
    in.endPoll();
    beginCase("下一帧：键盘与鼠标的 pressed 边沿都清零，held 保留");
    CHECK(!in.pressed(Key::W));
    CHECK(in.down(Key::W));
    CHECK(!in.mousePressed(MouseButton::Right));
    CHECK(in.mouseDown(MouseButton::Right));
}
