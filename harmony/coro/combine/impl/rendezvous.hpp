#pragma once

#include <atomic>
#include <cstddef>

namespace harmony::coro::impl {

class Rendezvous {
 public:
  bool CompleteResult() {
    return state_.fetch_or(HasResult) == StartedAllTasks;
  }

  bool DoneStartingTasks() {
    return state_.fetch_or(StartedAllTasks) == HasResult;
  }

 private:
  enum State : size_t {
    Empty = 0,
    HasResult = 1,
    StartedAllTasks = 2,
  };

 private:
  std::atomic<size_t> state_{Empty};
};

}  // namespace harmony::coro::impl
