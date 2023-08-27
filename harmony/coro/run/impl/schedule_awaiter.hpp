#pragma once

#include <coroutine>

#include <harmony/runtime/executors/interface.hpp>
#include <harmony/runtime/executors/task.hpp>

namespace harmony::coro::impl {

class ScheduleAwaiter : public runtime::executors::TaskBase {
 public:
  explicit ScheduleAwaiter(runtime::executors::IExecutor* executor);

  bool await_ready() noexcept;
  void await_suspend(std::coroutine_handle<> coroutine) noexcept;
  void await_resume() noexcept;

 public:
  void Run() noexcept override;

 private:
  runtime::executors::IExecutor* executor_{nullptr};
  std::coroutine_handle<> coroutine_{nullptr};
};

}  // namespace harmony::coro::impl
