#include <cassert>

#include <harmony/runtime/executors/compute/executor.hpp>

namespace harmony::executors {

ComputeExecutor::ComputeExecutor(size_t threads)
    : threads_count_(threads) {
}

ComputeExecutor::~ComputeExecutor() {
  assert(queue_.IsClosed());
}

void ComputeExecutor::WorkerRoutine() {
  while (auto task = queue_.TakeOne()) {
    task.value()->Run();
    planned_tasks_.Done();
  }
}

void ComputeExecutor::Submit(TaskBase* task, ExecutorHint) {
  planned_tasks_.Add(1);
  queue_.Push(task);
}

void ComputeExecutor::Start() {
  for (size_t i = 0; i < threads_count_; ++i) {
    workers_.emplace_back([this]() {
      WorkerRoutine();
    });
  }
}

void ComputeExecutor::Stop() {
  queue_.Close();

  for (auto& worker : workers_) {
    worker.join();
  }
}

void ComputeExecutor::WaitIdle() {
  planned_tasks_.Wait();
}

}  // namespace harmony::executors
