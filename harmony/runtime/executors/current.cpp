#include <harmony/runtime/executors/current.hpp>

namespace harmony::runtime::executors {

thread_local IExecutor* current_executor = nullptr;

void SetExecutorToTLS(IExecutor* executor) {
  current_executor = executor;
}

IExecutor* CurrentExecutor() {
  return current_executor;
}

}  // namespace harmony::runtime::executors
