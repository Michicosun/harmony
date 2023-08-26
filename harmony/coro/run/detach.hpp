#pragma once

#include <harmony/coro/run/impl/detach_task.hpp>

namespace harmony::coro {

template <concepts::Awaitable Awaitable>
inline void Detach(Awaitable&& object) {
  auto task = CreateDetachTask(std::forward<Awaitable>(object));
  task.Start();
}

}  // namespace harmony::coro
