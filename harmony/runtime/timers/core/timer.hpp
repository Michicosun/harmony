#pragma once

#include <harmony/runtime/event.hpp>
#include <harmony/runtime/timers/core/deadline.hpp>
#include <harmony/support/intrusive/forward_list.hpp>

namespace harmony::timers {

struct TimerBase : runtime::Event,
                   support::ForwardListNode<TimerBase> {
  uint64_t id;
  Deadline deadline;
};

}  // namespace harmony::timers
