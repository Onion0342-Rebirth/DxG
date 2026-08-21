#pragma once
#include "render/SceneRenderer.h"   // ISceneAssets
#include "render/Sprite.h"
#include "render/SpriteSheet.h"
#include "world/TileMap.h"
#include "ui/Font.h"
#include <array>

namespace d25 {

// 资源层：游戏里唯一的美术/数据入口。
//   - 持有地形纹理、物件精灵、角色精灵表、字体等强类型句柄；
//   - 实现 render::ISceneAssets，供 SceneRenderer 消费；
//   - 真正的"从 assets/ 读取 PNG/图集/Tiled JSON"留待后续接入。
//
// 约定：set 接口由启动时的资源构建流程调用（未来替换为文件加载）；
//      get 接口在运行期以 const 访问，缺失资源时返回 nullptr（渲染层做兜底）。
class ResourceManager : public ISceneAssets {
public:
    ResourceManager();

    // ---- ISceneAssets（渲染层只读访问） ----
    const Sprite* terrain(Terrain t) const override {
        return terrainSet_[size_t(t)] ? &terrain_[size_t(t)] : nullptr;
    }
    const Sprite* tree() const override { return treeSet_ ? &tree_ : nullptr; }
    const Sprite* rock() const override { return rockSet_ ? &rock_ : nullptr; }
    const SpriteSheet* playerSheet() const override {
        return playerSet_ ? &playerSheet_ : nullptr;
    }

    // ---- 写入（启动期注入） ----
    void setTerrain(Terrain t, const Sprite& s);
    void setTree(const Sprite& s);
    void setRock(const Sprite& s);
    void setPlayerSheet(const SpriteSheet& sheet);

    // 字体：默认内置一份，也可替换。
    const Font& hudFont() const { return font_; }
    void setFont(const Font& f) { font_ = f; }

    // 兜底字体（无需资源即可画 UI）。
    static const Font& fallbackFont();

private:
    std::array<Sprite, size_t(Terrain::Count)> terrain_;
    std::array<bool,   size_t(Terrain::Count)> terrainSet_{};
    Sprite tree_;
    bool treeSet_ = false;
    Sprite rock_;
    bool rockSet_ = false;
    SpriteSheet playerSheet_;
    bool playerSet_ = false;
    Font font_;
};

} // namespace d25
