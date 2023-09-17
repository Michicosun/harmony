#include <iostream>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.WithIO();
  scheduler.WithTimer();
  scheduler.Start();

  auto amain = [&]() -> coro::Task<> {
    co_await coro::Schedule(scheduler);

    std::string host;
    net::DnsResolver resolver;

    while (true) {
      std::cin >> host;
      auto info = co_await resolver.Resolve(host);
      std::cout << "> " << host << ": " << info.ip_address << std::endl;
    }

    co_return {};
  };

  coro::Run(amain());

  scheduler.Stop();
  return 0;
}
