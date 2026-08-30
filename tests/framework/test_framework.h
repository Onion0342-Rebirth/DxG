#pragma once
// 极简测试框架：仅提供断言计数与退出码。
// 用法：每个分类测试文件包含本头，实现自己的 runXxxTests()；test_main.cpp 顺序调用并汇总。
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>

namespace d25test {

// 全局计数（在 test_main.cpp 定义）。
extern int g_passCount;
extern int g_failCount;
int currentCaseFailures(); // test_main.cpp 中实现，用于标记用例失败

// 判断文件/目录是否存在。
inline bool fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

// 拼接路径片段。
inline std::string joinPath(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    const char last = a.back();
    if (last == '/' || last == '\\') return a + b;
    return a + "/" + b;
}

// 定位 assets 目录：兼容从仓库根 / DxG / DxG/build / tests 等不同工作目录运行；
// 也支持用环境变量 DXG_ASSETS_DIR 显式指定。找不到返回空串（调用方按 SKIP 处理）。
inline std::string findAssetsDir() {
    if (const char* env = std::getenv("DXG_ASSETS_DIR")) {
        if (fileExists(joinPath(env, "robot/stand/robot-stand-down.png"))) return env;
    }
    const char* candidates[] = {
        "assets",
        "../assets",
        "../../assets",
        "../../../assets",
        "DxG/assets",
        "../DxG/assets",
        "../../DxG/assets",
        "tests/../assets",
    };
    for (const char* c : candidates) {
        if (fileExists(joinPath(c, "robot/stand/robot-stand-down.png"))) return c;
    }
    return "";
}

inline void checkImpl(bool cond, const char* expr, const char* file, int line) {
    if (cond) {
        ++g_passCount;
    } else {
        ++g_failCount;
        std::printf("    [FAIL] %s  (%s:%d)\n", expr, file, line);
    }
}

// 打印用例名。
inline void beginCase(const std::string& name) {
    std::printf("  - %s\n", name.c_str());
}

} // namespace d25test

// 断言：条件为真计入通过，否则计入失败并打印位置。
#define CHECK(cond) d25test::checkImpl((cond), #cond, __FILE__, __LINE__)

// 打印分类标题。
#define SECTION(title) std::printf("[%s]\n", (title))

// 缺少素材时的统一跳过提示（不算失败）。
#define SKIP_REASON(reason) std::printf("    [SKIP] %s\n", (reason))
