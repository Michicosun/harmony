#pragma once

#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/io_request.hpp>
#include <harmony/runtime/scheduler.hpp>
#include <harmony/threads/event/event.hpp>

namespace harmony::coro::impl {

class [[nodiscard]] FdAwaiter : public executors::TaskBase,
                                public io::IORequest {
  struct FdStopHandler {
    FdAwaiter* fd_awaiter{nullptr};

    explicit FdStopHandler(FdAwaiter* fd_awaiter)
        : fd_awaiter{fd_awaiter} {
    }

    void operator()() {
      if (fd_awaiter->state.Cancel()) {
        auto& ios = fd_awaiter->parameters_->scheduler->GetIOEventSource();
        ios.DeleteIORequest(fd_awaiter);
      }
    }
  };

 public:
  explicit FdAwaiter(io::Fd fd, io::Operation operation);

  bool await_ready() noexcept;

  template <concepts::BasePromiseConvertible Promise>
  void await_suspend(std::coroutine_handle<Promise> coroutine) {
    BasePromise& promise = coroutine.promise();
    parameters_ = &promise.GetParameters();

    // check cancel request
    CheckCancel(parameters_);

    // check scheduler
    parameters_->CheckActiveScheduler();

    // save coro handle for resume
    coroutine_ = coroutine;

    // save fd in epoll and setup cancellation
    auto& ios = parameters_->scheduler->GetIOEventSource();
    ios.AddIORequest(this);

    stop_callback_.emplace(parameters_->stop_token, FdStopHandler(this));
    cb_constructed_.store(true);
  }

  io::EventStatus await_resume();

 public:
  void Schedule();
  void Run() noexcept override;

 public:
  void OnFinish() override;

 private:
  std::coroutine_handle<> coroutine_{nullptr};
  CoroParameters* parameters_{nullptr};

  std::atomic<bool> cb_constructed_{false};
  std::optional<std::stop_callback<FdStopHandler>> stop_callback_;
};

}  // namespace harmony::coro::impl
