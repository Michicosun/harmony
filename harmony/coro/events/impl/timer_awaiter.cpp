#include <harmony/coro/events/impl/timer_awaiter.hpp>

namespace harmony::coro::impl {

TimerAwaiter::TimerAwaiter(timers::Duration duration)
    : duration_(std::move(duration)) {
}

bool TimerAwaiter::await_ready() noexcept {
  return false;
}

void TimerAwaiter::await_resume() {
  CheckCancel(parameters_);
}

void TimerAwaiter::Schedule() {
  parameters_->scheduler->Schedule(this);
}

void TimerAwaiter::Run() noexcept {
  coroutine_.resume();
}

void TimerAwaiter::OnFinish() {
  cb_constructed_.Wait();
  stop_callback_.reset();
  Schedule();
}

}  // namespace harmony::coro::impl
