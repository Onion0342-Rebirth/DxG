#pragma once
#include <string>

namespace d25 {

class PixelBuffer;

// 把帧缓冲写成 PPM(P6) 文件（方便无头调试/出图）。
// 成功返回 true；失败返回 false 并把原因写入 err（可空）。
bool writePPM(const PixelBuffer& buf, const std::string& path, std::string* err = nullptr);

} // namespace d25
