#pragma once

#include <atomic>
#include <coroutine>
#include <optional>
#include <stop_token>

#include <harmony/coro/combine/impl/rendezvous.hpp>
#include <harmony/support/ref_counter/ref_counter.hpp>

namespace harmony::coro::impl {

class AddTaskAwaiter;
class GroupTask;

class GroupSharedState : public support::RefCounter {
  struct UpstreamStopHandler {
    GroupSharedState* shared_state{nullptr};

    explicit UpstreamStopHandler(GroupSharedState* shared_state)
        : shared_state{shared_state} {
    }

    void operator()() {
      shared_state->CancelAll();
    }
  };

 public:
  GroupSharedState();

  bool Register(std::coroutine_handle<> waiter, std::stop_token stop_token);

 public:
  AddTaskAwaiter AddTask(GroupTask&& task);

  void CompleteOne();
  void CancelAll();

 private:
  friend class AddTaskAwaiter;

  // sync logic
  Rendezvous consensus_;
  std::atomic<size_t> running_cnt_{0};

  // state
  std::coroutine_handle<> waiter_{nullptr};

  // cancellation
  std::stop_source stop_source_;
  std::optional<std::stop_callback<UpstreamStopHandler>> stop_callback_;
};

}  // namespace harmony::coro::impl
