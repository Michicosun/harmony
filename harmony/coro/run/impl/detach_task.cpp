#include <exception>

#include <harmony/coro/run/impl/detach_task.hpp>

namespace harmony::coro::impl {

bool DetachTask::DetachPromise::FinalAwaiter::await_ready() noexcept {
  return false;
}

void DetachTask::DetachPromise::FinalAwaiter::await_suspend(
    handle coroutine) noexcept {
  coroutine.destroy();
}

void DetachTask::DetachPromise::FinalAwaiter::await_resume() noexcept {
}

DetachTask DetachTask::DetachPromise::get_return_object() noexcept {
  return DetachTask(handle::from_promise(*this));
}

std::suspend_always DetachTask::DetachPromise::initial_suspend() noexcept {
  return {};
}

void DetachTask::DetachPromise::return_void() noexcept {
}

void DetachTask::DetachPromise::unhandled_exception() noexcept {
  std::terminate();
}

DetachTask::DetachPromise::FinalAwaiter
DetachTask::DetachPromise::final_suspend() noexcept {
  return FinalAwaiter{};
}

void DetachTask::Start() {
  coro_.resume();
}

DetachTask::DetachTask(std::coroutine_handle<promise_type> h) noexcept
    : coro_(h) {
}

}  // namespace harmony::coro::impl
