#pragma once

#include <harmony/coro/events/impl/timer_awaiter.hpp>

namespace harmony::coro {

inline auto SleepFor(timers::Duration timeout) noexcept {
  return impl::TimerAwaiter(std::move(timeout));
}

}  // namespace harmony::coro
