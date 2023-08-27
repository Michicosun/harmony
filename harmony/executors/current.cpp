#include <harmony/executors/current.hpp>

namespace harmony::executors {

thread_local IExecutor* current_executor = nullptr;

void SetExecutorToTLS(IExecutor* executor) {
  current_executor = executor;
}

IExecutor* CurrentExecutor() {
  return current_executor;
}

}  // namespace harmony::executors
