#pragma once
#include <cstdint>
#include <vector>
#include "core/Vec.h"

namespace d25 {

// 地形类型（渲染层决定纹理/颜色，逻辑层判定可走性）。
enum class Terrain : uint8_t {
    Grass = 0,
    Dirt,
    Stone,
    Water,
    Count
};

// 单个瓦片：高度（米）决定角色站到哪、地形决定可走性与外观。
struct Tile {
    Terrain terrain = Terrain::Grass;
    float height = 0.0f;
    bool blocked = false; // 额外阻挡标记（建筑/障碍占位，未来可扩展）
};

// 瓦片地图：逻辑层唯一的地形权威。
// 坐标约定：瓦片 (tx,tz) 覆盖世界 x∈[tx*ts,(tx+1)*ts]、z∈[tz*ts,(tz+1)*ts]。
class TileMap {
public:
    TileMap() = default;
    TileMap(int width, int height, float tileSize = 1.0f);

    void resize(int width, int height, float tileSize = 1.0f);

    int width() const { return width_; }
    int height() const { return height_; }
    float tileSize() const { return tileSize_; }

    bool inBounds(int tx, int tz) const;
    bool inBounds(float x, float z) const;

    Tile& at(int tx, int tz);
    const Tile& at(int tx, int tz) const;

    // 世界坐标处是否可走（水不可走，blocked 不可走）。
    bool isWalkable(float x, float z) const;
    // 世界坐标处的地形高度（双线性插值）。
    float heightAt(float x, float z) const;

private:
    int width_ = 0, height_ = 0;
    float tileSize_ = 1.0f;
    std::vector<Tile> tiles_;
};

} // namespace d25
