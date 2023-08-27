#pragma once

#include <harmony/executors/hint.hpp>
#include <harmony/executors/interface.hpp>
#include <harmony/executors/task.hpp>

namespace harmony::executors {

// emplaces pointer to executor to tls
void SetExecutorToTLS(IExecutor* executor);

// returns current executor,
// nullptr if running outside executor
IExecutor* CurrentExecutor();

// submits task to current executor if presents
// otherwise throws an error
void SubmitToCurrent(TaskBase* task,
                     ExecutorHint hint = ExecutorHint::Unspecified);

}  // namespace harmony::executors
