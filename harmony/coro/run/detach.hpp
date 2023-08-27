#pragma once

#include <harmony/coro/run/impl/detach_task.hpp>

namespace harmony::coro {

template <concepts::Awaitable Awaitable>
inline void Detach(Awaitable&& object) {
  auto task = impl::CreateDetachTask(std::forward<Awaitable>(object));
  task.Start();
}

template <class AwaitableConstructor>
inline auto Detach(AwaitableConstructor&& constructor) {
  return Detach(constructor());
}

}  // namespace harmony::coro
