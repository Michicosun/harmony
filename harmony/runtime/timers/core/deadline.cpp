#include <harmony/runtime/timers/core/deadline.hpp>

namespace harmony::timers {

Deadline DeadlineFromNow(Duration duration) {
  return Deadline{Clock::now() + duration};
}

}  // namespace harmony::timers
