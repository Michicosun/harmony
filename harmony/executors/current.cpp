#include <stdexcept>

#include <harmony/executors/current.hpp>

namespace harmony::executors {

thread_local IExecutor* current_executor = nullptr;

void SetExecutorToTLS(IExecutor* executor) {
  current_executor = executor;
}

IExecutor* CurrentExecutor() {
  return current_executor;
}

void SubmitToCurrent(TaskBase* task, ExecutorHint hint) {
  auto* cur_executor = CurrentExecutor();

  if (cur_executor == nullptr) {
    throw std::runtime_error(
        "to schedule a task, "
        "you need to be executed inside the executor");
  }

  cur_executor->Submit(task, hint);
}

}  // namespace harmony::executors
