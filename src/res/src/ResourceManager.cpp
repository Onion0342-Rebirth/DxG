#include "res/ResourceManager.h"

namespace d25 {

ResourceManager::ResourceManager() = default;

void ResourceManager::setTerrain(Terrain t, const Sprite& s) {
    terrain_[size_t(t)] = s;
    terrainSet_[size_t(t)] = true;
}

void ResourceManager::setTree(const Sprite& s) {
    tree_ = s;
    treeSet_ = true;
}

void ResourceManager::setRock(const Sprite& s) {
    rock_ = s;
    rockSet_ = true;
}

void ResourceManager::setPlayerSheet(const SpriteSheet& sheet) {
    playerSheet_ = sheet;
    playerSet_ = true;
}

const Font& ResourceManager::fallbackFont() {
    static const Font f;
    return f;
}

} // namespace d25
