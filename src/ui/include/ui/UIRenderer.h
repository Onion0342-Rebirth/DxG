#pragma once
#include "core/Color.h"
#include "ui/Font.h"
#include <string>

namespace d25 {

class PixelBuffer;

// 2D UI 绘制：把 5x7 位图字体以指定像素放大倍数画到帧缓冲上（HUD/暂停面板用）。
// 所有方法为 const：不持有可变状态，只写 PixelBuffer。
class UIRenderer {
public:
    explicit UIRenderer(const Font& font, int pixelScale = 2)
        : font_(font), pixelScale_(pixelScale > 0 ? pixelScale : 1) {}

    void setPixelScale(int s) { pixelScale_ = s > 0 ? s : 1; }
    int pixelScale() const { return pixelScale_; }

    int textWidth(const std::string& text) const;
    int textHeight() const { return Font::kGlyphH * pixelScale_; }

    void drawText(PixelBuffer& buf, int x, int y, const std::string& text, Color color) const;

    void drawRect(PixelBuffer& buf, int x, int y, int w, int h, Color color) const;

    // 简单面板：1px 边框 + 内部填充。
    void drawPanel(PixelBuffer& buf, int x, int y, int w, int h,
                   Color fill, Color border) const;

private:
    const Font& font_;
    int pixelScale_;
};

} // namespace d25
