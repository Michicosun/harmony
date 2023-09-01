#pragma once

#include <harmony/coro/combine/impl/all/all_awaiter.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro {

template <concepts::Awaitable... Awaitables>
inline auto All(Awaitables&&... awaitables) {
  return impl::AllAwaiter<
      typename traits::AwaitableTraits<Awaitables>::AwaiterReturnT...>(
      impl::CreateAllTask(std::forward<Awaitables>(awaitables))...);
}

}  // namespace harmony::coro
