#pragma once

#include <harmony/coro/core/task.hpp>

namespace harmony::coro {

template <class T>
inline void Detach(Task<T>&& task) {
  task.ReleaseCoroutine().resume();
}

}  // namespace harmony::coro
