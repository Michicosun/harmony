#pragma once

#include <harmony/coro/run/impl/schedule_awaiter.hpp>
#include <harmony/runtime/executors/interface.hpp>

namespace harmony::coro {

inline auto Schedule(runtime::executors::IExecutor& executor) {
  return impl::ScheduleAwaiter(&executor);
}

}  // namespace harmony::coro
