#pragma once
#include <memory>
#include <vector>

namespace d25 {

class Game;

// 游戏状态基类：玩法/菜单/过场都派生自它。
// 同一时刻只有栈顶状态被更新与渲染；压栈会暂停旧栈顶，弹栈会恢复新栈顶。
class GameState {
public:
    explicit GameState(Game& game) : game_(&game) {}
    virtual ~GameState() = default;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onPause() {}   // 被新状态盖住
    virtual void onResume() {}  // 新状态弹出后恢复

    virtual void update(float dt) = 0;
    virtual void render() = 0;

protected:
    Game& game() const { return *game_; }

private:
    Game* game_;
};

// 状态栈：拥有状态（unique_ptr）。push 时旧顶 onPause -> 新顶 onEnter；
// pop 时当前顶 onExit -> 新顶 onResume。
class StateStack {
public:
    void push(std::unique_ptr<GameState> state);
    void pop();
    void clear();

    GameState* top() const { return stack_.empty() ? nullptr : stack_.back().get(); }
    bool empty() const { return stack_.empty(); }
    std::size_t size() const { return stack_.size(); }

private:
    std::vector<std::unique_ptr<GameState>> stack_;
};

} // namespace d25
