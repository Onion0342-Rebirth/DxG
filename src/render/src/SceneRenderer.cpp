#include "render/SceneRenderer.h"
#include "core/PixelBuffer.h"
#include "core/DepthBuffer.h"
#include "render/Sprite.h"
#include "render/SpriteSheet.h"
#include "world/World.h"
#include "world/entity/Entity.h"
#include "world/character/Character.h"
#include "world/player/Player.h"
#include <algorithm>
#include <cmath>

namespace d25 {

namespace {
constexpr float kGroundExtent = 300.f;
}

SceneRenderer::SceneRenderer(const World& world, Camera& camera, const ISceneAssets& assets)
    : world_(world), cam_(camera), assets_(assets) {}

float SceneRenderer::viewDepthKey(const Mat4& view, const Vec3f& worldPos) {
    const Vec4f c = Mat4::mul(view, {worldPos.x, worldPos.y, worldPos.z, 1.f});
    return -c.z; // 视空间 depth（越大越远）
}

void SceneRenderer::render(PixelBuffer& color, DepthBuffer& depth) {
    color_ = &color;
    depth_ = &depth;

    color.fill({0, 0, 0, 255});
    depth.clear();

    ras_.begin(color, depth);
    // 远景雾与地平线同色，让瓷砖/物件自然融入天空。
    ras_.setFog({228, 240, 250}, 18.f, 90.f);

    drawSky(color);
    drawGround();
    drawTiles();
    drawActors();
}

void SceneRenderer::drawSky(PixelBuffer& color) {
    const Color top{116, 176, 232, 255};
    const Color horizon{228, 240, 250, 255};
    for (int y = 0; y < color.height(); ++y) {
        const float t = float(y) / float(color.height());
        const Color c = Color::lerp(top, horizon, t);
        for (int x = 0; x < color.width(); ++x) {
            color.set(x, y, c);
        }
    }
}

void SceneRenderer::drawGround() {
    // 一大片地面兜底（地图外区域），颜色与草色接近。
    const Vec3f corners[4] = {
        {-kGroundExtent, 0.f, -kGroundExtent},
        { kGroundExtent, 0.f, -kGroundExtent},
        { kGroundExtent, 0.f,  kGroundExtent},
        {-kGroundExtent, 0.f,  kGroundExtent},
    };
    const float uv[8] = {0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f};
    drawWorldQuad(corners, uv, nullptr, {76, 158, 63, 255});
}

void SceneRenderer::drawTiles() {
    const TileMap& map = world_.map();
    const float ts = map.tileSize();

    for (int tz = 0; tz < map.height(); ++tz) {
        for (int tx = 0; tx < map.width(); ++tx) {
            const Tile& tile = map.at(tx, tz);
            const float x0 = float(tx) * ts;
            const float z0 = float(tz) * ts;
            const float x1 = x0 + ts;
            const float z1 = z0 + ts;
            const float y = tile.height;

            const Vec3f corners[4] = {
                {x0, y, z0}, {x1, y, z0}, {x1, y, z1}, {x0, y, z1},
            };
            const float uv[8] = {0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f};
            // 棋盘格明暗交替：相邻瓦片一亮一暗，纯色地面也能看出玩家移动。
            // 暗格统一压到 0.82 倍亮度（无纹理时压兜底色，有纹理时压 tint 白值）。
            const bool dark = ((tx + tz) & 1) != 0;
            const Sprite* tex = assets_.terrain(tile.terrain);
            Color tint = tex ? Color{255, 255, 255, 255} : terrainFallback(tile.terrain);
            if (dark) tint = shade(tint, 0.82f);
            drawWorldQuad(corners, uv, tex, tint);
        }
    }
}

void SceneRenderer::drawActors() {
    const Mat4& view = cam_.view();
    const TileMap& map = world_.map();
    std::vector<DrawItem> items;

    // 静态实体。
    for (const Entity& e : world_.entities()) {
        const float h = map.heightAt(e.pos.x, e.pos.y);
        const Vec3f base{e.pos.x, h, e.pos.y};
        const float key = viewDepthKey(view, base);

        if (e.kind == EntityKind::Tree) {
            const Sprite* tex = assets_.tree();
            items.push_back({key, [this, base, tex, s = e.scale] {
                drawBillboard(base, 1.6f * s, 2.9f * s, tex, {255, 255, 255, 255});
            }});
        } else if (e.kind == EntityKind::Rock) {
            const Sprite* tex = assets_.rock();
            items.push_back({key, [this, base, tex, s = e.scale] {
                drawBillboard(base, 1.0f * s, 0.7f * s, tex, {200, 200, 200, 255});
            }});
        } else if (e.kind == EntityKind::House) {
            items.push_back({key, [this, base] {
                drawBox({base.x - 1.5f, base.y, base.z - 1.5f},
                        {base.x + 1.5f, base.y + 2.0f, base.z + 1.5f},
                        {170, 130, 90, 255}, {190, 150, 110, 255});
                drawBox({base.x - 1.6f, base.y + 2.0f, base.z - 1.6f},
                        {base.x + 1.6f, base.y + 2.6f, base.z + 1.6f},
                        {150, 80, 60, 255}, {170, 90, 70, 255});
            }});
        } else if (e.kind == EntityKind::Crate) {
            items.push_back({key, [this, base] {
                drawBox({base.x - 0.4f, base.y, base.z - 0.4f},
                        {base.x + 0.4f, base.y + 0.8f, base.z + 0.4f},
                        {150, 110, 70, 255}, {170, 130, 90, 255});
            }});
        }
    }

    // 角色（玩家 + 非玩家角色）。玩家单独持有于 World，NPC 在 characters() 中，
    // 两者都按 Character 接口取位移动画帧，加入同一 depth 排序列表。
    const SpriteSheet* sheet = assets_.playerSheet();
    auto addCharacter = [&](const Character& c) {
        const Vec2f& p = c.pos();
        const float h = map.heightAt(p.x, p.y);
        const Vec3f base{p.x, h, p.y};
        const float key = viewDepthKey(view, base);

        // 注意：framePtr 必须按值捕获，不能取局部引用变量的地址（旧版本有悬挂指针 bug）。
        const Sprite* framePtr = nullptr;
        float w = 0.8f, hh = 0.8f;
        if (sheet && !sheet->empty()) {
            const int idx = c.animFrame();
            const Sprite& fr = sheet->frame(std::clamp(idx, 0, sheet->frameCount() - 1));
            framePtr = &fr;
            w = 0.8f;
            hh = w * (fr.height() / float(std::max(1, fr.width())));
        }
        // 公告板为屏幕对齐（水平/竖直边沿相机基向量），正方形精灵帧投影后仍为正方形，
        // 无需按俯角做高度补偿。
        items.push_back({key, [this, base, framePtr, w, hh] {
            drawBillboard(base, w, hh, framePtr, {255, 255, 255, 255});
        }});
    };
    if (const Player* p = world_.player()) addCharacter(*p);
    for (const Character& c : world_.characters()) addCharacter(c);

    // 远 -> 近 排序（descending depth）。
    std::stable_sort(items.begin(), items.end(),
                     [](const DrawItem& a, const DrawItem& b) { return a.depth > b.depth; });
    for (const DrawItem& it : items) it.draw();
}

void SceneRenderer::drawWorldQuad(const Vec3f corners[4], const float uv[8],
                                  const Sprite* tex, Color tint) {
    if (!color_ || !depth_) return;

    // 把世界点变换到相机空间，再投影（相机空间才能算透视校正量）。
    const Mat4& view = cam_.view();
    Vec3f cam[4];
    for (int i = 0; i < 4; ++i) {
        const Vec4f c = Mat4::mul(view, {corners[i].x, corners[i].y, corners[i].z, 1.f});
        cam[i] = {c.x, c.y, c.z};
    }

    ScreenVertex sv[4];
    for (int i = 0; i < 4; ++i) {
        if (!cam_.projectCamUV(cam[i], uv[i * 2], uv[i * 2 + 1], sv[i])) return;
    }

    // 两个三角形：0,1,2 与 0,2,3。
    ras_.triangle(sv[0], sv[1], sv[2], tex, tint);
    ras_.triangle(sv[0], sv[2], sv[3], tex, tint);
}

void SceneRenderer::billboardCorners(const Vec3f& rightDir, const Vec3f& upDir,
                                     const Vec3f& base, float w, float h, Vec3f out[4]) {
    // 屏幕对齐公告板：水平边对齐相机右向量、竖直边对齐相机上向量（均为世界系单位
    // 基向量）。二者在相机空间恰为 ±X / ±Y，因此无论角色在屏幕什么位置，投影后
    // 水平边严格水平、竖直边严格竖直，不会歪斜；且同深度下屏幕高宽比恒等于 w/h，
    // 正方形精灵帧不会因俯视透视被压扁。
    const Vec3f right = rightDir.normalized();
    const Vec3f up = upDir.normalized();
    const Vec3f hw = right * (w * 0.5f);
    const Vec3f vh = up * h;
    const Vec3f bl = base - hw;
    const Vec3f br = base + hw;
    // 输出顺序：左上、右上、右下、左下（与 drawWorldQuad 的 UV 约定一致）。
    out[0] = bl + vh;
    out[1] = br + vh;
    out[2] = br;
    out[3] = bl;
}

void SceneRenderer::drawBillboard(const Vec3f& base, float w, float h,
                                  const Sprite* tex, Color tint) {
    if (!color_ || !depth_) return;

    Vec3f corners[4];
    billboardCorners(cam_.rightWorld(), cam_.upWorld(), base, w, h, corners);
    const float uv[8] = {0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f};
    drawWorldQuad(corners, uv, tex, tint);
}

void SceneRenderer::drawBox(const Vec3f& mn, const Vec3f& mx, Color side, Color top) {
    if (!color_ || !depth_) return;

    // 5 个面（底面不画）。
    struct Face { Vec3f c[4]; Color color; };
    const Face faces[5] = {
        // top
        {{ {mn.x, mx.y, mn.z}, {mx.x, mx.y, mn.z}, {mx.x, mx.y, mx.z}, {mn.x, mx.y, mx.z} }, top},
        // front (-Z)
        {{ {mn.x, mn.y, mn.z}, {mx.x, mn.y, mn.z}, {mx.x, mx.y, mn.z}, {mn.x, mx.y, mn.z} }, shade(side, 0.85f)},
        // back (+Z)
        {{ {mx.x, mn.y, mx.z}, {mn.x, mn.y, mx.z}, {mn.x, mx.y, mx.z}, {mx.x, mx.y, mx.z} }, shade(side, 0.75f)},
        // left (-X)
        {{ {mn.x, mn.y, mx.z}, {mn.x, mn.y, mn.z}, {mn.x, mx.y, mn.z}, {mn.x, mx.y, mx.z} }, shade(side, 0.80f)},
        // right (+X)
        {{ {mx.x, mn.y, mn.z}, {mx.x, mn.y, mx.z}, {mx.x, mx.y, mx.z}, {mx.x, mx.y, mn.z} }, shade(side, 0.90f)},
    };

    const float uv[8] = {0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f};
    for (const Face& f : faces) {
        drawWorldQuad(f.c, uv, nullptr, f.color);
    }
}

Color SceneRenderer::shade(Color c, float factor) {
    return Color{
        uint8_t(std::clamp(int(c.r * factor), 0, 255)),
        uint8_t(std::clamp(int(c.g * factor), 0, 255)),
        uint8_t(std::clamp(int(c.b * factor), 0, 255)),
        c.a,
    };
}

Color SceneRenderer::terrainFallback(Terrain t) {
    switch (t) {
        case Terrain::Grass: return {76, 158, 63, 255};
        case Terrain::Dirt:  return {138, 106, 63, 255};
        case Terrain::Stone: return {125, 125, 125, 255};
        case Terrain::Water: return {58, 111, 196, 255};
        default:             return {255, 0, 255, 255};
    }
}

} // namespace d25
