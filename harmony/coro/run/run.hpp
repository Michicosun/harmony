#pragma once

#include <harmony/coro/run/impl/run_task.hpp>

namespace harmony::coro {

template <concepts::Awaitable Awaitable>
inline auto Run(Awaitable&& object) {
  threads::MPSCEvent event;

  auto task = impl::CreateRunTask(std::forward<Awaitable>(object));
  task.Start(event);

  event.Wait();

  return task.UnwrapResult();
}

}  // namespace harmony::coro
