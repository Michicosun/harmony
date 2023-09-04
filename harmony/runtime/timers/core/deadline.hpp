#pragma once

#include <chrono>

namespace harmony::timers {

using namespace std::chrono_literals;

using Clock = std::chrono::steady_clock;
using Deadline = std::chrono::time_point<Clock>;
using Duration = std::chrono::milliseconds;

Deadline DeadlineFromNow(Duration duration);

}  // namespace harmony::timers
