#include <cassert>

#include <harmony/coro/run/impl/schedule_awaiter.hpp>
#include <harmony/runtime/executors/interface.hpp>

namespace harmony::coro::impl {

ScheduleAwaiter::ScheduleAwaiter(runtime::executors::IExecutor* executor)
    : executor_{executor} {
  assert(executor_);
}

bool ScheduleAwaiter::await_ready() noexcept {
  return false;
}

void ScheduleAwaiter::await_suspend(
    std::coroutine_handle<> coroutine) noexcept {
  coroutine_ = coroutine;
  executor_->Submit(this);
}

void ScheduleAwaiter::await_resume() noexcept {
}

void ScheduleAwaiter::Run() noexcept {
  coroutine_.resume();
}

}  // namespace harmony::coro::impl
