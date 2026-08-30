// 测试总入口：一次性顺序运行所有分类测试并汇总结果（回归测试）。
// 不依赖 SDL2，仅链接 dxg_core（core/anim/world/render/ui/res/game 中非平台部分）。
//
// 分类约定：tests/<模块>/test_*.cpp 各自实现 run<...>Tests()，在此声明并调用。
#include "framework/test_framework.h"

#ifdef _WIN32
#include <windows.h>
#endif

// 测试框架全局计数的唯一定义（声明在 test_framework.h）。
namespace d25test {
int g_passCount = 0;
int g_failCount = 0;
int currentCaseFailures() { return g_failCount; }
} // namespace d25test

// 各分类测试入口（定义在 tests/<模块>/test_*.cpp）。
void runImagePngTests();
void runMat4Tests();
void runPlayerAssetsTests();
void runCharacterMoveTests();
void runPlayerTests();
void runRenderSmokeTests();
void runFollowCamTests();
void runBillboardBoardTests();
void runMouseInputTests();
void runPauseMenuTests();

#ifdef _WIN32
// Windows 控制台默认代码页多为 GBK(936)，而程序输出为 UTF-8，会导致中文乱码。
// 启动时把输入/输出代码页都切到 UTF-8(65001)。
static void enableUtf8Console() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
}
#endif

int main() {
#ifdef _WIN32
    enableUtf8Console();
#endif
    std::printf("========== DxG 回归测试 ==========\n");

    SECTION("core");   runImagePngTests();
    SECTION("core");   runMat4Tests();
    SECTION("res");    runPlayerAssetsTests();
    SECTION("world");  runCharacterMoveTests();
    SECTION("world");  runPlayerTests();
    SECTION("render"); runRenderSmokeTests();
    SECTION("render"); runFollowCamTests();
    SECTION("render"); runBillboardBoardTests();
    SECTION("input");  runMouseInputTests();
    SECTION("game");   runPauseMenuTests();

    std::printf("----------------------------------\n");
    std::printf("结果：通过 %d，失败 %d\n", d25test::g_passCount, d25test::g_failCount);
    if (d25test::g_failCount == 0) {
        std::printf("全部通过\n");
        return 0;
    }
    std::printf("存在失败用例\n");
    return 1;
}
