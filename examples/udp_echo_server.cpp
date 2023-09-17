#include <iostream>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.WithIO();
  scheduler.Start();

  auto amain = [&]() -> coro::Task<> {
    co_await coro::Schedule(scheduler);

    std::string buffer(1024, 0);
    net::UdpSocket socket(net::AddressFamily::IPv4);
    socket.Bind(2020);

    while (true) {
      auto info = co_await socket.Receive(buffer);

      std::cout << info.sender.ip_address << ":" << info.sender.port << " > "
                << buffer.substr(0, info.size) << std::endl;

      co_await socket.Send(info.sender, buffer, info.size);
    }

    co_return {};
  };

  coro::Run(amain());

  scheduler.Stop();
  return 0;
}
