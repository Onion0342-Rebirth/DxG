#include "world/terrain/TileMap.h"
#include <algorithm>
#include <cmath>

namespace d25 {

TileMap::TileMap(int width, int height, float tileSize) {
    resize(width, height, tileSize);
}

void TileMap::resize(int width, int height, float tileSize) {
    width_ = std::max(0, width);
    height_ = std::max(0, height);
    tileSize_ = tileSize > 0.f ? tileSize : 1.f;
    tiles_.assign(size_t(width_) * size_t(height_), Tile{});
}

bool TileMap::inBounds(int tx, int tz) const {
    return tx >= 0 && tz >= 0 && tx < width_ && tz < height_;
}

bool TileMap::inBounds(float x, float z) const {
    return x >= 0.f && z >= 0.f && x < width_ * tileSize_ && z < height_ * tileSize_;
}

Tile& TileMap::at(int tx, int tz) {
    return tiles_[size_t(tz) * size_t(width_) + size_t(tx)];
}

const Tile& TileMap::at(int tx, int tz) const {
    return tiles_[size_t(tz) * size_t(width_) + size_t(tx)];
}

bool TileMap::isWalkable(float x, float z) const {
    if (!inBounds(x, z)) return false;
    const int tx = int(std::floor(x / tileSize_));
    const int tz = int(std::floor(z / tileSize_));
    if (!inBounds(tx, tz)) return false;
    const Tile& t = at(tx, tz);
    if (t.blocked) return false;
    return t.terrain != Terrain::Water;
}

float TileMap::heightAt(float x, float z) const {
    if (width_ <= 0 || height_ <= 0) return 0.f;
    const float fx = x / tileSize_;
    const float fz = z / tileSize_;
    int tx = int(std::floor(fx));
    int tz = int(std::floor(fz));
    const float u = fx - float(tx);
    const float v = fz - float(tz);

    // 边缘 clamp，保证边界外高度等于最近瓦片。
    const int tx0 = std::clamp(tx, 0, width_ - 1);
    const int tz0 = std::clamp(tz, 0, height_ - 1);
    const int tx1 = std::clamp(tx + 1, 0, width_ - 1);
    const int tz1 = std::clamp(tz + 1, 0, height_ - 1);

    const float h00 = at(tx0, tz0).height;
    const float h10 = at(tx1, tz0).height;
    const float h01 = at(tx0, tz1).height;
    const float h11 = at(tx1, tz1).height;

    const float h0 = h00 + (h10 - h00) * u;
    const float h1 = h01 + (h11 - h01) * u;
    return h0 + (h1 - h0) * v;
}

} // namespace d25
