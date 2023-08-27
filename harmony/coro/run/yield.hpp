#pragma once

#include <stdexcept>

#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/run/impl/schedule_awaiter.hpp>

namespace harmony::coro {

inline auto Yield() {
  if (executors::CurrentExecutor() == nullptr) {
    throw std::runtime_error("can't yield without executor");
  }

  return impl::ScheduleAwaiter(executors::CurrentExecutor());
}

}  // namespace harmony::coro
