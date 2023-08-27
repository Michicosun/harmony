#pragma once

#include <harmony/executors/interface.hpp>

namespace harmony::executors {

// emplaces pointer to executor to tls
void SetExecutorToTLS(IExecutor* executor);

// returns current executor,
// nullptr if running outside executor
IExecutor* CurrentExecutor();

}  // namespace harmony::executors
