#include <harmony/coro/events/impl/fd_awaiter.hpp>
#include <thread>

namespace harmony::coro::impl {

FdAwaiter::FdAwaiter(io::Fd fd, io::Operation operation) {
  this->fd = fd;
  this->operation = operation;
}

bool FdAwaiter::await_ready() noexcept {
  return false;
}

io::EventStatus FdAwaiter::await_resume() {
  CheckCancel(parameters_);
  return status;
}

void FdAwaiter::Schedule() {
  parameters_->scheduler->Schedule(this);
}

void FdAwaiter::Run() noexcept {
  coroutine_.resume();
}

void FdAwaiter::OnFinish() {
  while (!cb_constructed_.load()) {
  }

  stop_callback_.reset();
  Schedule();
}

}  // namespace harmony::coro::impl
