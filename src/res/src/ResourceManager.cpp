#include "res/ResourceManager.h"
#include "core/ImageIO.h"
#include <string>

namespace d25 {

namespace {

// 拼接路径片段：base 末尾有无分隔符都能正确拼出。
std::string joinPath(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    const char last = a.back();
    if (last == '/' || last == '\\') return a + b;
    return a + "/" + b;
}

// 把一张横向排列的精灵表（等尺寸正方形帧、单排）切成 frameCount 个 Sprite。
// frameSize 为单帧边长（像素）；切出来的帧宽=高=frameSize。
std::vector<Sprite> sliceSheet(const ImageData& img, int frameSize, int frameCount) {
    std::vector<Sprite> frames;
    frames.reserve(frameCount);
    for (int f = 0; f < frameCount; ++f) {
        Sprite s(frameSize, frameSize);
        const int x0 = f * frameSize;
        for (int y = 0; y < frameSize; ++y) {
            for (int x = 0; x < frameSize; ++x) {
                const uint8_t* p = img.pixelAt(x0 + x, y);
                if (!p) continue; // 理论不会发生（切帧范围已校验）
                s.setPixel(x, y, Color{p[0], p[1], p[2], p[3]});
            }
        }
        frames.push_back(std::move(s));
    }
    return frames;
}

// 水平镜像一帧（左<->右）：用于由"朝右"素材生成"朝左"，省去一份镜像素材。
Sprite flipHorizontal(const Sprite& src) {
    const int w = src.width();
    const int h = src.height();
    Sprite dst(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            dst.setPixel(w - 1 - x, y, src.pixel(x, y));
        }
    }
    return dst;
}

} // namespace

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

bool ResourceManager::loadPlayerRobot(const std::string& assetsDir, std::string* err) {
    // 素材约定（Aseprite 横向导出，RGBA8888，单帧 32x32，每张 6 帧）：
    //   robot/stand/robot-stand-up.png          -> North（上 / -Z）
    //   robot/stand/robot-stand-down.png        -> South（下 / +Z）
    //   robot/stand/robot_stand_right.png       -> East（右 / +X）
    //   robot/stand/robot-stand-up-right.png    -> NorthEast（右上 / +X -Z）
    //   robot/stand/robot-stand-down-left.png   -> SouthWest（左下 / -X +Z）
    // 其余三个方向不存素材，由已有素材逐帧水平镜像得到：
    //   West（左）<- East、NorthWest（左上）<- NorthEast、SouthEast（右下）<- SouthWest。
    const int kFrameSize = 32;   // 单帧边长（像素）
    const int kFramesPerDir = 6; // 每个方向 6 帧
    // 动画节奏（帧/秒）：动画播放器按时间推进、与渲染帧率解耦，节奏只由 fps 决定。
    // walk 放慢到 6fps（6 帧一轮 = 1.0 秒，走路沉稳）；idle 用更慢的 2fps 循环同一套
    // 6 帧（一轮 = 3.0 秒），呈现待机/呼吸感，而不是僵在首帧。
    const float kWalkFps = 6.f;
    const float kIdleFps = 2.f;

    struct DirSource {
        Direction dir;
        std::string fileName;
    };
    const DirSource sources[5] = {
        {Direction::North,     "robot/stand/robot-stand-up.png"},
        {Direction::South,     "robot/stand/robot-stand-down.png"},
        {Direction::East,      "robot/stand/robot_stand_right.png"},
        {Direction::NorthEast, "robot/stand/robot-stand-up-right.png"},
        {Direction::SouthWest, "robot/stand/robot-stand-down-left.png"},
    };

    constexpr int kDirCount = int(Direction::Count); // 8
    std::array<std::vector<Sprite>, kDirCount> dirFrames;
    for (const DirSource& src : sources) {
        const std::string path = joinPath(assetsDir, src.fileName);
        ImageData img;
        std::string localErr;
        if (!loadPNG(path, img, &localErr)) {
            if (err) *err = localErr;
            return false;
        }
        const int expectedW = kFrameSize * kFramesPerDir;
        if (img.width != expectedW || img.height != kFrameSize) {
            if (err) {
                *err = "角色精灵表尺寸不符（期望 " + std::to_string(expectedW) + "x" +
                       std::to_string(kFrameSize) + "）: " + path;
            }
            return false;
        }
        dirFrames[size_t(src.dir)] = sliceSheet(img, kFrameSize, kFramesPerDir);
    }

    // 三个缺失方向 = 对应方向的逐帧水平镜像（左 <-> 右，上下不变，斜向正好互为镜像）。
    const struct MirrorPair { Direction dst; Direction src; } mirrors[3] = {
        {Direction::West,      Direction::East},      // 左 <- 右
        {Direction::NorthWest, Direction::NorthEast}, // 左上 <- 右上
        {Direction::SouthEast, Direction::SouthWest}, // 右下 <- 左下
    };
    for (const MirrorPair& mp : mirrors) {
        dirFrames[size_t(mp.dst)].reserve(kFramesPerDir);
        for (const Sprite& f : dirFrames[size_t(mp.src)]) {
            dirFrames[size_t(mp.dst)].push_back(flipHorizontal(f));
        }
    }

    // 按 Direction 枚举顺序（N/NE/E/SE/S/SW/W/NW）拼出 48 帧精灵表
    // （基址 = (int)dir * kFramesPerDir）。
    SpriteSheet sheet;
    for (int d = 0; d < kDirCount; ++d) {
        for (Sprite& f : dirFrames[d]) {
            sheet.add(std::move(f));
        }
    }
    setPlayerSheet(sheet);

    // 构建 8 方向 x (idle/walk) 共 16 个剪辑：
    //   walk 与 idle 都循环播放该方向的 6 帧，区别只在节奏（fps）：
    //   walk=6fps 走路，idle=2fps 更慢的待机呼吸；Character 对两者都按时间推进。
    for (int d = 0; d < kDirCount; ++d) {
        const int base = d * kFramesPerDir;
        playerWalk_[d] = AnimationClip{base, kFramesPerDir, kWalkFps, true};
        playerIdle_[d] = AnimationClip{base, kFramesPerDir, kIdleFps, true};
    }
    playerClipsSet_ = true;
    return true;
}

const Font& ResourceManager::fallbackFont() {
    static const Font f;
    return f;
}

} // namespace d25
