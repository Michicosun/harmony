#include <harmony/threads/futex/futex.hpp>
#include <harmony/threads/wait_group/wait_group.hpp>

namespace harmony::threads {

void WaitGroup::Add(size_t count) {
  size_.fetch_add(count);
}

void WaitGroup::Done() {
  auto prev_size = size_.fetch_sub(1);

  if (prev_size == 1) {
    auto wake_key = threads::futex::PrepareWake(size_);
    threads::futex::WakeAll(wake_key);
  }
}

void WaitGroup::Wait() {
  uint32_t cur_size = size_.load();

  while (cur_size > 0) {
    threads::futex::Wait(size_, cur_size);
    cur_size = size_.load();
  }
}

}  // namespace harmony::threads
