#include <harmony/threads/futex.hpp>

#include <harmony/support/event/event.hpp>

namespace harmony::support {

void MPSCEvent::Wait() {
  threads::futex::Wait(completed_, States::Empty);
}

void MPSCEvent::Complete() {
  auto wake_key = threads::futex::PrepareWake(completed_);

  completed_.store(States::Completed);
  threads::futex::WakeOne(wake_key);
}

void MPSCEvent::Reset() {
  completed_.store(States::Empty);
}

}  // namespace harmony::support
