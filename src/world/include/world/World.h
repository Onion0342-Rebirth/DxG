#pragma once
#include "world/terrain/TileMap.h"
#include "world/entity/Entity.h"
#include "world/character/Character.h"
#include "world/player/Player.h"
#include <memory>
#include <vector>

namespace d25 {

// 世界：拥有地图、静态实体、玩家与非玩家角色集合；是玩法层唯一的数据容器。
// 具体规则（生成、AI、交互）由 GameplayState 驱动；World 只负责持有数据与逐帧推进。
//
// 子包划分：
//   world/terrain   地形（TileMap）
//   world/entity    静态物件（Entity）
//   world/character 所有角色的通用能力（Character 基类、Direction）
//   world/player    玩家角色（Player : Character）
class World {
public:
    World() = default;
    World(int width, int height, float tileSize = 1.0f) : map_(width, height, tileSize) {}

    TileMap& map() { return map_; }
    const TileMap& map() const { return map_; }

    std::vector<Entity>& entities() { return entities_; }
    const std::vector<Entity>& entities() const { return entities_; }

    // 非玩家角色（NPC、怪物等）。
    std::vector<Character>& characters() { return characters_; }
    const std::vector<Character>& characters() const { return characters_; }

    // 玩家由玩法层创建并注入；未注入时为 nullptr。
    void setPlayer(std::unique_ptr<Player> p) { player_ = std::move(p); }
    Player* player() { return player_.get(); }
    const Player* player() const { return player_.get(); }

    void addEntity(const Entity& e) { entities_.push_back(e); }
    void addCharacter(const Character& c) { characters_.push_back(c); }

    // 推进玩家与所有非玩家角色（未来在此处加 AI、交互、机关）。
    void update(float dt);

private:
    TileMap map_{40, 40, 1.0f};
    std::vector<Entity> entities_;
    std::unique_ptr<Player> player_;
    std::vector<Character> characters_;
};

} // namespace d25
