#include <harmony/threads/event/event.hpp>
#include <harmony/threads/futex/futex.hpp>

namespace harmony::threads {

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

}  // namespace harmony::threads
