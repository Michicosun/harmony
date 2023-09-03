#pragma once

#include <harmony/coro/combine/impl/first/first_awaiter.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro {

template <concepts::Awaitable FirstAwaitable, concepts::Awaitable... Other>
inline auto First(FirstAwaitable&& first, Other&&... other) {
  using ReturnType = traits::AwaitableTraits<FirstAwaitable>::AwaiterReturnT;

  return impl::FirstAwaiter<ReturnType, impl::FirstTask<ReturnType>,
                            impl::FirstTask<typename traits::AwaitableTraits<
                                Other>::AwaiterReturnT>...>(
      impl::CreateFirstTask(std::forward<FirstAwaitable>(first)),
      impl::CreateFirstTask(std::forward<Other>(other))...);
}

}  // namespace harmony::coro
