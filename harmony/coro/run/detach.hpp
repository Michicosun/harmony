#pragma once

#include <harmony/coro/run/impl/detach_task.hpp>

namespace harmony::coro {

template <concepts::Awaitable awaitable>
inline void Detach(awaitable&& object) {
  auto task = CreateDetachTask(std::forward<awaitable>(object));
  task.Start();
}

}  // namespace harmony::coro
