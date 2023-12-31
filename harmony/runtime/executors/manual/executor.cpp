#include <cassert>
#include <cstddef>

#include <harmony/runtime/executors/manual/executor.hpp>

namespace harmony::executors {

ManualExecutor::ManualExecutor() {
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

void ManualExecutor::Start() {
}

void ManualExecutor::Stop() {
  assert(IsEmpty());
}

void ManualExecutor::WaitIdle() {
  assert(IsEmpty());
}

}  // namespace harmony::executors
