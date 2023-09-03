#include <cassert>

#include <harmony/coro/core/task_parameters.hpp>

namespace harmony::coro {

void CoroParameters::MergeFrom(const CoroParameters& other) {
  scheduler = other.scheduler;
  stop_token = other.stop_token;
}

bool CoroParameters::NeedCancel() const {
  return stop_token.stop_requested();
}

void CoroParameters::CheckCancel() const {
  if (stop_token.stop_requested()) {
    ThrowCancel();
  }
}

void CoroParameters::CheckActiveScheduler() const {
  assert(scheduler);
}

void CheckCancel(CoroParameters* parameters) {
  if (parameters != nullptr) {
    parameters->CheckCancel();
  }
}

void ThrowCancel() {
  throw Cancelled{};
}

}  // namespace harmony::coro
