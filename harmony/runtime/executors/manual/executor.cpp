#include <cstddef>

#include <harmony/runtime/executors/current.hpp>
#include <harmony/runtime/executors/manual/executor.hpp>

namespace harmony::runtime::executors {

ManualExecutor::ManualExecutor() {
  SetExecutorToTLS(this);  // creator of manual can run tasks
}

void ManualExecutor::Submit(TaskBase* task, ExecutorHint) {
  tasks_.PushBack(task);
}

void ManualExecutor::RunOne() {
  auto task = tasks_.PopFront();
  task->Run();
}

bool ManualExecutor::RunNext() {
  return Run(1) == 1;
}

size_t ManualExecutor::Run(size_t limit) {
  size_t completed = 0;

  while (completed < limit && !IsEmpty()) {
    RunOne();
    ++completed;
  }

  return completed;
}

size_t ManualExecutor::Drain() {
  size_t completed = 0;

  while (!IsEmpty()) {
    RunOne();
    ++completed;
  }

  return completed;
}

size_t ManualExecutor::TasksInQueue() const {
  return tasks_.Size();
}

bool ManualExecutor::IsEmpty() const {
  return tasks_.IsEmpty();
}

}  // namespace harmony::runtime::executors
