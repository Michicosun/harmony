#pragma once

#include <utility>

#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/concepts/awaiter.hpp>

namespace harmony::coro::traits {

template <concepts::Awaitable awaitable, typename = void>
struct AwaitableTraits {};

template <concepts::Awaitable awaitable>
static auto GetAwaiter(awaitable&& value) {
  return std::forward<awaitable>(value).operator co_await();
}

template <concepts::Awaiter awaiter>
static auto CompleteAwaiter(awaiter&& value) {
  return std::forward<awaiter>(value).await_resume();
}

template <concepts::Awaitable awaitable>
struct AwaitableTraits<awaitable> {
  using AwaiterT = decltype(GetAwaiter(std::declval<awaitable>()));
  using AwaiterReturnT = decltype(CompleteAwaiter(std::declval<AwaiterT>()));
};

}  // namespace harmony::coro::traits
