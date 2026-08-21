#include "ui/UIRenderer.h"
#include "core/PixelBuffer.h"
#include <cstring>

namespace d25 {

int UIRenderer::textWidth(const std::string& text) const {
    if (text.empty()) return 0;
    const int s = pixelScale_;
    return int(text.size()) * Font::kAdvance * s - (Font::kAdvance - Font::kGlyphW) * s;
}

void UIRenderer::drawText(PixelBuffer& buf, int x, int y,
                          const std::string& text, Color color) const {
    const int s = pixelScale_;
    int cursorX = x;
    for (char ch : text) {
        const uint8_t* glyph = Font::glyph(ch);
        for (int row = 0; row < Font::kGlyphH; ++row) {
            const uint8_t bits = glyph[row];
            for (int col = 0; col < Font::kGlyphW; ++col) {
                // bit4 = 最左列。
                if (bits & (1 << (Font::kGlyphW - 1 - col))) {
                    for (int dy = 0; dy < s; ++dy) {
                        for (int dx = 0; dx < s; ++dx) {
                            buf.set(cursorX + col * s + dx, y + row * s + dy, color);
                        }
                    }
                }
            }
        }
        cursorX += Font::kAdvance * s;
    }
}

void UIRenderer::drawRect(PixelBuffer& buf, int x, int y, int w, int h, Color color) const {
    for (int yy = y; yy < y + h; ++yy) {
        for (int xx = x; xx < x + w; ++xx) {
            buf.set(xx, yy, color);
        }
    }
}

void UIRenderer::drawPanel(PixelBuffer& buf, int x, int y, int w, int h,
                            Color fill, Color border) const {
    drawRect(buf, x, y, w, h, border);
    drawRect(buf, x + 1, y + 1, w - 2, h - 2, fill);
}

} // namespace d25
