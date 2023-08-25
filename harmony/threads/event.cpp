#include <harmony/threads/event.hpp>

namespace harmony::threads::sync {

void SPSCEvent::Wait() {
  futex::Wait(completed_, States::Empty);
}

void SPSCEvent::Complete() {
  auto wake_key = futex::PrepareWake(completed_);

  completed_.store(States::Completed);
  futex::WakeOne(wake_key);
}

void SPSCEvent::Reset() {
  completed_.store(States::Empty);
}

}  // namespace harmony::threads::sync
