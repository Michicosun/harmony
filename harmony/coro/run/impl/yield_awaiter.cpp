#include <harmony/coro/run/impl/yield_awaiter.hpp>

namespace harmony::coro::impl {

bool YieldAwaiter::await_ready() noexcept {
  return false;
}

void YieldAwaiter::await_resume() noexcept {
  parameters_->CheckCancel();
}

void YieldAwaiter::Run() noexcept {
  coroutine_.resume();
}

}  // namespace harmony::coro::impl
