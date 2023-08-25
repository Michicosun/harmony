#pragma once

#include <harmony/coro/run/impl/run_task.hpp>
#include <harmony/support/event.hpp>

namespace harmony::coro {

template <concepts::Awaitable awaitable>
auto Run(awaitable&& object) {
  support::MPSCEvent event;

  auto task = CreateRunTask(std::forward<awaitable>(object));
  task.Start(event);

  event.Wait();

  return task.UnwrapResult();
}

}  // namespace harmony::coro
