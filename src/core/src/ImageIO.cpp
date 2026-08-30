#include "core/ImageIO.h"
#include "core/PixelBuffer.h"
#include <fstream>
#include <string>

// stb_image：公有领域单头文件图像库，vendored 于 src/core/third_party/stb/。
// 仅在本编译单元展开其实现（STB_IMAGE_IMPLEMENTATION），其余文件只通过 ImageIO.h 使用。
// 这里关闭它在 -Wall/-Wextra（或 MSVC /W4）下的大量第三方告警，避免污染本项目的告警等级。
#if defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wall"
  #pragma GCC diagnostic ignored "-Wextra"
  #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  #pragma GCC diagnostic ignored "-Wsign-compare"
  #pragma GCC diagnostic ignored "-Wunused-function"
  #pragma GCC diagnostic ignored "-Wcast-qual"
#elif defined(_MSC_VER)
  #pragma warning(push, 0)
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG   // 本项目只需要解码 PNG，裁掉其余格式以减小体积
#include "../third_party/stb/stb_image.h"
#if defined(__GNUC__)
  #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
  #pragma warning(pop)
#endif

namespace d25 {

bool writePPM(const PixelBuffer& buf, const std::string& path, std::string* err) {
    if (buf.empty()) {
        if (err) *err = "empty buffer";
        return false;
    }
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        if (err) *err = "cannot open file: " + path;
        return false;
    }
    out << "P6\n" << buf.width() << ' ' << buf.height() << "\n255\n";
    for (int y = 0; y < buf.height(); ++y) {
        for (int x = 0; x < buf.width(); ++x) {
            const Color c = buf.get(x, y);
            out.put(char(c.r));
            out.put(char(c.g));
            out.put(char(c.b));
        }
    }
    return static_cast<bool>(out);
}

bool loadPNG(const std::string& path, ImageData& out, std::string* err) {
    out = ImageData{};

    int w = 0, h = 0, channelsInFile = 0;
    // 最后一个参数 4 = 强制输出 RGBA 4 通道，与 d25::Color 的 r,g,b,a 布局一致。
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channelsInFile, 4);
    if (!data) {
        const char* reason = stbi_failure_reason();
        if (err) {
            *err = "PNG 解码失败: " + path + " (" + (reason ? reason : "未知原因") + ")";
        }
        return false;
    }

    out.width = w;
    out.height = h;
    out.rgba.assign(data, data + static_cast<size_t>(w) * h * 4);
    stbi_image_free(data);
    return true;
}

} // namespace d25
