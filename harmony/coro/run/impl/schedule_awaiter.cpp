#include <cassert>

#include <harmony/coro/run/impl/schedule_awaiter.hpp>

namespace harmony::coro::impl {

ScheduleAwaiter::ScheduleAwaiter(runtime::IScheduler* scheduler)
    : scheduler_{scheduler} {
  assert(scheduler);
}

bool ScheduleAwaiter::await_ready() noexcept {
  return false;
}

void ScheduleAwaiter::await_resume() noexcept {
}

void ScheduleAwaiter::Run() noexcept {
  coroutine_.resume();
}

}  // namespace harmony::coro::impl
