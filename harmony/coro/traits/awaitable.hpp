#pragma once

#include <utility>

#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/concepts/awaiter.hpp>

namespace harmony::coro::traits {

template <concepts::Awaitable Awaitable, typename = void>
struct AwaitableTraits {};

template <concepts::Awaitable Awaitable>
static auto GetAwaiter(Awaitable&& value) {
  return std::forward<Awaitable>(value).operator co_await();
}

template <concepts::Awaiter Awaiter>
static auto CompleteAwaiter(Awaiter&& value) {
  return std::forward<Awaiter>(value).await_resume();
}

template <concepts::Awaitable Awaitable>
struct AwaitableTraits<Awaitable> {
  using AwaiterT = decltype(GetAwaiter(std::declval<Awaitable>()));
  using AwaiterReturnT = decltype(CompleteAwaiter(std::declval<AwaiterT>()));
};

}  // namespace harmony::coro::traits
