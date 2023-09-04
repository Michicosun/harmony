#pragma once

#include <harmony/runtime/event_sm.hpp>

namespace harmony::runtime {

struct Event {
  virtual ~Event() = default;
  virtual void OnFinish() = 0;

  EventStateMachine state_;
};

}  // namespace harmony::runtime
