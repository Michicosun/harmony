#pragma once

#include <harmony/coro/run/impl/schedule_awaiter.hpp>

namespace harmony::coro {

inline auto Schedule(runtime::IScheduler& executor) {
  return impl::ScheduleAwaiter(&executor);
}

}  // namespace harmony::coro
