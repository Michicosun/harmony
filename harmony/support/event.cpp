#include <harmony/threads/futex.hpp>

#include <harmony/support/event.hpp>

namespace harmony::support {

void SPSCEvent::Wait() {
  threads::futex::Wait(completed_, States::Empty);
}

void SPSCEvent::Complete() {
  auto wake_key = threads::futex::PrepareWake(completed_);

  completed_.store(States::Completed);
  threads::futex::WakeOne(wake_key);
}

void SPSCEvent::Reset() {
  completed_.store(States::Empty);
}

}  // namespace harmony::support
