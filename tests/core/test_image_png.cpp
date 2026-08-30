// 分类：core
// 验证 PNG 解码（core/ImageIO::loadPNG）。
// 需要真实素材；定位不到 assets 时按 SKIP 处理（保证无素材环境也能跑完回归）。
#include "framework/test_framework.h"
#include "core/ImageIO.h"
#include <string>

void runImagePngTests() {
    using namespace d25test;

    beginCase("loadPNG 解码 robot-stand-down.png");
    const std::string assets = findAssetsDir();
    if (assets.empty()) {
        SKIP_REASON("未找到 assets 目录（设置环境变量 DXG_ASSETS_DIR 可指定），跳过 PNG 解码用例");
        return;
    }

    {
        d25::ImageData img;
        std::string err;
        const std::string path = joinPath(assets, "robot/stand/robot-stand-down.png");
        const bool ok = d25::loadPNG(path, img, &err);
        CHECK(ok);
        if (ok) {
            CHECK(img.ok());
            CHECK(img.width == 192);
            CHECK(img.height == 32);
            CHECK(img.rgba.size() == size_t(192) * 32 * 4);
        }
    }

    beginCase("loadPNG 三方向精灵表均为 192x32");
    {
        const char* files[] = {
            "robot/stand/robot-stand-up.png",
            "robot/stand/robot-stand-down.png",
            "robot/stand/robot_stand_right.png",
        };
        int okCount = 0;
        for (const char* f : files) {
            d25::ImageData img;
            if (d25::loadPNG(joinPath(assets, f), img)) {
                if (img.width == 192 && img.height == 32) ++okCount;
            }
        }
        CHECK(okCount == 3);
    }

    beginCase("loadPNG 不存在的文件返回 false 且不崩溃");
    {
        d25::ImageData img;
        std::string err;
        const bool ok = d25::loadPNG(joinPath(assets, "robot/stand/__not_exist__.png"), img, &err);
        CHECK(!ok);
        CHECK(!img.ok());
        CHECK(!err.empty());
    }
}
