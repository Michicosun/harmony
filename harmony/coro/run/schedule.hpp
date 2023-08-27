#pragma once

#include <harmony/coro/run/impl/schedule_awaiter.hpp>
#include <harmony/executors/interface.hpp>

namespace harmony::coro {

inline auto Schedule(executors::IExecutor& executor) {
  return impl::ScheduleAwaiter(&executor);
}

}  // namespace harmony::coro
