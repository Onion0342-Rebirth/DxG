#include "game/GameState.h"

namespace d25 {

void StateStack::push(std::unique_ptr<GameState> state) {
    if (!state) return;
    if (!stack_.empty()) stack_.back()->onPause();
    stack_.push_back(std::move(state));
    stack_.back()->onEnter();
}

void StateStack::pop() {
    if (stack_.empty()) return;
    stack_.back()->onExit();
    stack_.pop_back();
    if (!stack_.empty()) stack_.back()->onResume();
}

void StateStack::clear() {
    while (!stack_.empty()) pop();
}

} // namespace d25
