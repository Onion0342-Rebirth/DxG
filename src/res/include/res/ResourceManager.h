#pragma once
#include "render/SceneRenderer.h"   // ISceneAssets
#include "render/Sprite.h"
#include "render/SpriteSheet.h"
#include "world/terrain/TileMap.h"
#include "world/character/Direction.h"
#include "anim/AnimationClip.h"
#include "ui/Font.h"
#include <array>
#include <string>

namespace d25 {

// 资源层：游戏里唯一的美术/数据入口。
//   - 持有地形纹理、物件精灵、角色精灵表、角色动画剪辑、字体等强类型句柄；
//   - 实现 render::ISceneAssets，供 SceneRenderer 消费；
//   - 角色精灵表/剪辑可从 assets/ 目录加载（loadPlayerRobot），其余地形/物件仍为内存注入。
//
// 约定：set / load 接口由启动时的资源构建流程调用；
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

    // ---- 角色素材加载（从 assets/ 目录） ----
    // 加载 robot 五方向（上/下/右/右上/左下）移动精灵表：解码 PNG、按正方形帧切分、
    // 拼为 48 帧玩家精灵表（按 Direction 枚举顺序 N/NE/E/SE/S/SW/W/NW 各 6 帧），
    // 另外三个方向由已有素材逐帧水平镜像得到：West←East、NorthWest←NorthEast、SouthEast←SouthWest；
    // 同时构建 8 方向 x (idle/walk) 共 16 个剪辑。
    // assetsDir 为 assets 根目录（其下应有 robot/stand/...）。
    // 成功返回 true；任一文件缺失/解码失败返回 false 并写 err（已加载内容不保证有效，调用方可走兜底）。
    bool loadPlayerRobot(const std::string& assetsDir, std::string* err = nullptr);

    // 角色动画剪辑访问（供 Character::setAnimClips 注入）；未加载时返回 nullptr。
    const AnimationClip* playerIdleClip(Direction d) const {
        return playerClipsSet_ ? &playerIdle_[size_t(d)] : nullptr;
    }
    const AnimationClip* playerWalkClip(Direction d) const {
        return playerClipsSet_ ? &playerWalk_[size_t(d)] : nullptr;
    }

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

    // 角色动画剪辑：下标 = (int)Direction（N/NE/E/SE/S/SW/W/NW）。
    std::array<AnimationClip, size_t(Direction::Count)> playerIdle_{};
    std::array<AnimationClip, size_t(Direction::Count)> playerWalk_{};
    bool playerClipsSet_ = false;

    Font font_;
};

} // namespace d25
