#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace d25 {

class PixelBuffer;

// 解码后的原始图像：紧密排列的 8 位 RGBA 像素（行优先，自上而下，每行 width*4 字节）。
// core 层只产出原始像素，不知道 render::Sprite 的存在，保持 core 不依赖 render。
struct ImageData {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> rgba;

    bool ok() const { return width > 0 && height > 0 && rgba.size() == size_t(width) * height * 4; }

    // 取 (x,y) 处像素的 4 通道指针；越界返回 nullptr。
    const uint8_t* pixelAt(int x, int y) const {
        if (x < 0 || y < 0 || x >= width || y >= height) return nullptr;
        return &rgba[size_t(y) * width * 4 + size_t(x) * 4];
    }
};

// 把帧缓冲写成 PPM(P6) 文件（方便无头调试/出图）。
// 成功返回 true；失败返回 false 并把原因写入 err（可空）。
bool writePPM(const PixelBuffer& buf, const std::string& path, std::string* err = nullptr);

// 从文件解码 PNG（内部用 vendored stb_image，强制输出 4 通道 RGBA）。
// 成功返回 true 并填充 out；失败返回 false 并把原因写入 err（可空），out 被清空。
bool loadPNG(const std::string& path, ImageData& out, std::string* err = nullptr);

} // namespace d25
