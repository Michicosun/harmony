#include <array>
#include <cstddef>
#include <iostream>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT

coro::Task<> worker(net::AcceptInfo ai) {
  auto& ci = ai.con_info;
  std::cout << "connected " << ci.ip_address << ":" << ci.port << std::endl;

  net::TcpSocket socket(ai.fd);
  std::array<std::byte, 1024> buffer;

  while (true) {
    size_t n = co_await socket.ReadSome(buffer);
    co_await socket.WriteSome(buffer, n);
  }

  co_return {};
}

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.WithIO();
  scheduler.Start();

  auto amain = [&]() -> coro::Task<> {
    co_await coro::Schedule(scheduler);

    net::TcpAcceptor acceptor(2020);
    coro::TaskGroup tg;

    while (true) {
      auto con = co_await acceptor.Accept();
      co_await tg.Start(worker(std::move(con)));
    }

    co_await tg.Wait();
    co_return {};
  };

  coro::Run(amain());

  scheduler.Stop();
  return 0;
}
