#pragma once

#include <stdexcept>
#include <variant>

#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/core/task.hpp>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/coro/run/impl/schedule_awaiter.hpp>

namespace harmony::coro {

inline Task<std::monostate> Yield() {
  auto params = co_await kThisCoro;

  if (params.executor == nullptr) {
    throw std::runtime_error("can't yield without executor");
  }

  co_await ScheduleAwaiter(params.executor);
  co_return std::monostate{};
}

}  // namespace harmony::coro
