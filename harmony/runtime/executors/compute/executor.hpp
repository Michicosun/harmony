#pragma once

#include <thread>
#include <vector>

#include <harmony/runtime/executors/hint.hpp>
#include <harmony/runtime/executors/task.hpp>
#include <harmony/support/queues/blocking_queue.hpp>
#include <harmony/threads/wait_group/wait_group.hpp>

namespace harmony::executors {

class ComputeExecutor {
 public:
  explicit ComputeExecutor(size_t threads);
  ~ComputeExecutor();

  void Submit(TaskBase* task, ExecutorHint = ExecutorHint::Unspecified);

  void Start();
  void Stop();
  void WaitIdle();

 private:
  void WorkerRoutine();

 private:
  size_t threads_count_;
  std::vector<std::thread> workers_;
  threads::WaitGroup planned_tasks_;
  support::UnboundedBlockingQueue<TaskBase> queue_;
};

}  // namespace harmony::executors
