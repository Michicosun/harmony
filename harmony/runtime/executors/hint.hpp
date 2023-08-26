#pragma once

namespace harmony::runtime::executors {

enum class ExecutorHint {
  Unspecified,  // random
  Lifo = 2,     // use LIFO scheduling
  Yield = 3,    // use global queue
};

}  // namespace harmony::runtime::executors
