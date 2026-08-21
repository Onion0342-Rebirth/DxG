#include "core/ImageIO.h"
#include "core/PixelBuffer.h"
#include <fstream>

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

} // namespace d25
