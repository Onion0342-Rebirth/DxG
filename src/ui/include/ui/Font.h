#pragma once
#include <cstdint>

namespace d25 {

// 内置 5x7 位图字体：
//   每个字形 7 字节，每字节低 5 位为一行像素（bit4=最左列，bit0=最右列）。
//   字符前进宽度 6 像素（5 像素字形 + 1 像素间距）。
//   覆盖 A-Z、0-9、空格及 ! - . / : ；其它字符回退为 '?'。
class Font {
public:
    static constexpr int kGlyphW = 5;
    static constexpr int kGlyphH = 7;
    static constexpr int kAdvance = 6; // 含 1px 间距

    // 取字符字形（7 字节，低 5 位有效）；未知字符返回 fallback '?'。
    static const uint8_t* glyph(char c);

    // 归一化：小写转大写，非 ASCII 或不支持字符 -> '?'。
    static char normalize(char c);
};

} // namespace d25
