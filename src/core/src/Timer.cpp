#include "core/Timer.h"
#include <chrono>

namespace d25 {

namespace {
double nowSecImpl() {
    using namespace std::chrono;
    return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
}
} // namespace

Timer::Timer(float fixedDt) : fixedDt_(fixedDt), last_(nowSecImpl()) {}

bool Timer::step() {
    const double now = nowSecImpl();
    const double elapsed = now - last_;
    last_ = now;
    accumulator_ += elapsed;
    frameDt_ = float(elapsed);

    if (accumulator_ >= fixedDt_) {
        accumulator_ -= fixedDt_;
        return true;
    }
    return false;
}

double Timer::nowSec() { return nowSecImpl(); }

} // namespace d25
