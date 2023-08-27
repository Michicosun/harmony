#pragma once

#include <harmony/runtime/executors/interface.hpp>

namespace harmony::runtime::executors {

// emplaces pointer to executor to tls
void SetExecutorToTLS(IExecutor* executor);

// returns current executor,
// nullptr if running outside executor
IExecutor* CurrentExecutor();

}  // namespace harmony::runtime::executors
