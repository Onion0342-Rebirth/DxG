# DxG

2.5D 固定视角像素游戏框架，纯 C++17，C++ 软件光栅化，SDL2 仅用于窗口/输入/上屏。

这是一个**基础框架**：模块划分、主循环、状态栈、输入、2D 玩法逻辑、3D 软渲染、UI、资源容器都已就位，但**不包含正式场景/关卡内容**。具体游戏在 `game/GameplayState` 基础上扩展。

## 目录结构

每个模块下 `.h` 与 `.cpp` 分开放置：

```
src/
  <module>/
    include/<module>/*.h   # 模块公开头
    src/*.cpp              # 模块实现
```

包含方式：`#include "core/Vec.h"`、`#include "world/character/Character.h"`（每个模块的 `include/` 目录都在包含路径上；world 等模块内部按子包组织，如 `world/terrain/TileMap.h`、`world/player/Player.h`）。

| 模块 | 职责 |
| --- | --- |
| `core` | 数学 Vec/Mat/Color/Random、PixelBuffer/DepthBuffer、PPM 输出、固定步长 Timer、Game 主循环骨架 |
| `input` | 抽象 Key、InputEvent、玩法 Action、InputManager（键位映射 + 三态查询） |
| `anim` | AnimationClip / AnimationPlayer（纯头文件，与渲染解耦） |
| `world` | 玩法数据层（内含子包）：terrain 地形 TileMap、entity 静态物件、character 角色通用能力（Character 基类/Direction）、player 玩家角色（Player : Character）；根目录 World 为世界容器 |
| `render` | Camera、Sprite/SpriteSheet、Rasterizer 软件三角形（透视校正 + z-buffer + 雾）、SceneRenderer |
| `ui` | 内置 5×7 位图 Font、UIRenderer、HUD |
| `res` | ResourceManager：全游戏唯一资源容器，实现 render 侧 `ISceneAssets` 接口 |
| `platform` | Platform 抽象 + SdlPlatform（唯一接触 SDL 的文件） |
| `game` | GameState/StateStack、GameApp 组合根、GameplayState、PauseState |

## 依赖方向

```
core ← input/anim ← world ← render/ui ← res ← game
platform → core + input
game → 全部
```

`render` 不反向依赖 `res`：渲染侧定义 `ISceneAssets` 端口，`ResourceManager` 实现它。

## 构建

### CMake

```powershell
cmake -S . -B build
cmake --build build
```

`dxg_core` 静态库（除 SDL 平台外的全部代码）**不需要 SDL2** 即可编译；
`dxg` 可执行文件需要 SDL2（`find_package(SDL2)`）。

### 直接用 g++（build.ps1）

```powershell
.\build.ps1                       # 用默认 -lSDL2
.\build.ps1 -SDL C:\path\to\SDL2  # 指定 SDL2 安装前缀
```

`dxg_core.a` 总会被构建；没有 SDL2 时可执行文件链接会失败并给出提示。

## 扩展点

- 在 `GameplayState::onEnter` 中摆放地图/实体/角色（当前是 40×40 空草地 + 一个可移动角色）。
- 资源从 `assets/` 加载的逻辑接入 `ResourceManager`（setter 已就绪，文件加载待实现）。
- 新状态派生 `GameState` 并通过 `GameApp::states().push(...)` 入栈。
