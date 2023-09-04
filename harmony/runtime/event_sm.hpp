#pragma once

#include <atomic>
#include <cstddef>

namespace harmony::runtime {

class EventStateMachine {
 public:
  bool Cancel() {
    State empty_state = Empty;
    return state_.compare_exchange_strong(empty_state, Cancelled);
  }

  bool Finish() {
    State empty_state = Empty;
    return state_.compare_exchange_strong(empty_state, Finished);
  }

 private:
  enum State : size_t {
    Empty = 0,
    Cancelled = 1,
    Finished = 2,
  };

 private:
  std::atomic<State> state_{Empty};
};

}  // namespace harmony::runtime
