#include "input/InputManager.h"

namespace d25 {

void InputManager::bindDefaults() {
    mapKey(Action::MoveUp, Key::W);
    mapKey(Action::MoveUp, Key::Up);
    mapKey(Action::MoveDown, Key::S);
    mapKey(Action::MoveDown, Key::Down);
    mapKey(Action::MoveLeft, Key::A);
    mapKey(Action::MoveLeft, Key::Left);
    mapKey(Action::MoveRight, Key::D);
    mapKey(Action::MoveRight, Key::Right);

    mapKey(Action::Interact, Key::X);
    mapKey(Action::Interact, Key::Space);

    mapKey(Action::Confirm, Key::Enter);
    mapKey(Action::Confirm, Key::Space);

    mapKey(Action::Cancel, Key::Backspace);
    mapKey(Action::Pause, Key::Escape);
    mapKey(Action::Quit, Key::Q);
}

void InputManager::mapKey(Action action, Key key) {
    const size_t a = size_t(action);
    if (a >= size_t(Action::Count) || key == Key::None) return;
    auto& cnt = bindingCount_[a];
    if (cnt >= kMaxKeysPerAction) return;
    bindings_[a][size_t(cnt)] = key;
    ++cnt;
}

void InputManager::beginFrame() {
    keysPressed_.fill(false);
    keysReleased_.fill(false);
    mousePressed_.fill(false);
    mouseReleased_.fill(false);
}

void InputManager::handleEvent(const InputEvent& ev) {
    switch (ev.type) {
        case InputEvent::Type::MouseMove:
            mouseX_ = ev.mouseX;
            mouseY_ = ev.mouseY;
            return;

        case InputEvent::Type::MouseDown:
        case InputEvent::Type::MouseUp: {
            const size_t b = size_t(ev.button);
            if (b >= size_t(MouseButton::Count)) return;
            mouseX_ = ev.mouseX;
            mouseY_ = ev.mouseY;
            const bool isDown = ev.type == InputEvent::Type::MouseDown;
            if (isDown) {
                if (!mouseDown_[b]) {
                    mouseDown_[b] = true;
                    mousePressed_[b] = true;
                }
            } else {
                if (mouseDown_[b]) {
                    mouseDown_[b] = false;
                    mouseReleased_[b] = true;
                }
            }
            return;
        }

        case InputEvent::Type::KeyDown:
        case InputEvent::Type::KeyUp:
            break;  // 键盘事件走下面的既有逻辑

        default:
            return;  // None/Quit 等与按键状态无关
    }

    const size_t k = size_t(ev.key);
    if (k >= size_t(Key::Count)) return;

    if (ev.type == InputEvent::Type::KeyDown) {
        if (!keysDown_[k]) {
            keysDown_[k] = true;
            keysPressed_[k] = true;
        }
    } else if (ev.type == InputEvent::Type::KeyUp) {
        if (keysDown_[k]) {
            keysDown_[k] = false;
            keysReleased_[k] = true;
        }
    }
}

void InputManager::endPoll() {}

bool InputManager::down(Key key) const { return keysDown_[size_t(key)]; }
bool InputManager::pressed(Key key) const { return keysPressed_[size_t(key)]; }
bool InputManager::released(Key key) const { return keysReleased_[size_t(key)]; }

bool InputManager::down(Action action) const {
    for (int i = 0; i < bindingCount_[size_t(action)]; ++i)
        if (keysDown_[size_t(bindings_[size_t(action)][size_t(i)])]) return true;
    return false;
}

bool InputManager::pressed(Action action) const {
    for (int i = 0; i < bindingCount_[size_t(action)]; ++i)
        if (keysPressed_[size_t(bindings_[size_t(action)][size_t(i)])]) return true;
    return false;
}

bool InputManager::released(Action action) const {
    for (int i = 0; i < bindingCount_[size_t(action)]; ++i)
        if (keysReleased_[size_t(bindings_[size_t(action)][size_t(i)])]) return true;
    return false;
}

bool InputManager::mouseDown(MouseButton button) const {
    const size_t b = size_t(button);
    return b < size_t(MouseButton::Count) && mouseDown_[b];
}

bool InputManager::mousePressed(MouseButton button) const {
    const size_t b = size_t(button);
    return b < size_t(MouseButton::Count) && mousePressed_[b];
}

bool InputManager::mouseReleased(MouseButton button) const {
    const size_t b = size_t(button);
    return b < size_t(MouseButton::Count) && mouseReleased_[b];
}

} // namespace d25
