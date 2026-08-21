#pragma once
#include "world/TileMap.h"
#include "world/Entity.h"
#include "world/Character.h"
#include <vector>

namespace d25 {

// 世界：拥有地图、实体、角色集合；是玩法层唯一的数据容器。
// 具体规则（生成、AI、交互）由 GameplayState 驱动；World 只负责持有数据与逐帧推进。
class World {
public:
    World() = default;
    World(int width, int height, float tileSize = 1.0f) : map_(width, height, tileSize) {}

    TileMap& map() { return map_; }
    const TileMap& map() const { return map_; }

    std::vector<Entity>& entities() { return entities_; }
    const std::vector<Entity>& entities() const { return entities_; }

    std::vector<Character>& characters() { return characters_; }
    const std::vector<Character>& characters() const { return characters_; }

    // 主角色约定为第一个角色（可为空指针）。
    Character* player() { return characters_.empty() ? nullptr : &characters_.front(); }
    const Character* player() const { return characters_.empty() ? nullptr : &characters_.front(); }

    void addEntity(const Entity& e) { entities_.push_back(e); }
    void addCharacter(const Character& c) { characters_.push_back(c); }

    // 推进所有角色（未来在此处加 AI、交互、机关）。
    void update(float dt);

private:
    TileMap map_{40, 40, 1.0f};
    std::vector<Entity> entities_;
    std::vector<Character> characters_;
};

} // namespace d25
