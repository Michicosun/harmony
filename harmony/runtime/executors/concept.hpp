#pragma once

#include <concepts>

#include <harmony/runtime/executors/hint.hpp>
#include <harmony/runtime/executors/task.hpp>

namespace harmony::executors {

template <class T>
concept Executor = requires(T type, TaskBase* task, ExecutorHint hint) {
  { type.Submit(task, hint) } -> std::same_as<void>;
  { type.WaitIdle() } -> std::same_as<void>;
};

}
