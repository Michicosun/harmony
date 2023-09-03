#pragma once

#include <exception>
#include <stop_token>

#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro {

struct Cancelled : public std::exception {
  using std::exception::exception;
};

class ThisCoroType {};
constexpr ThisCoroType kThisCoro;

struct CoroParameters {
  runtime::IScheduler* scheduler{nullptr};
  std::stop_token stop_token;

  void MergeFrom(const CoroParameters& other);

  bool NeedCancel() const;
  void CheckCancel() const;

  void CheckActiveScheduler() const;
};

void CheckCancel(CoroParameters* parameters);
void ThrowCancel();

}  // namespace harmony::coro
