#include <cassert>

#include <harmony/coro/core/task_parameters.hpp>

namespace harmony::coro {

void CoroParameters::CheckCancel() const {
  if (stop_token.stop_requested()) {
    throw Cancelled{};
  }
}

void CoroParameters::CheckActiveScheduler() const {
  assert(scheduler);
}

void CoroParameters::MergeFrom(const CoroParameters& other) {
  scheduler = other.scheduler;
  stop_token = other.stop_token;
}

void CheckCancel(CoroParameters* parameters) {
  if (parameters != nullptr) {
    parameters->CheckCancel();
  }
}

}  // namespace harmony::coro
