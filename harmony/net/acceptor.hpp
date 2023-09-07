#pragma once

#include <harmony/coro/core/task.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

struct AcceptInfo {
  io::Fd fd;
  std::string ip_address;
  size_t port;
};

class Acceptor {
 public:
  explicit Acceptor(size_t port, AddressFamily af = AddressFamily::IPv4);

  ~Acceptor();

  coro::Task<AcceptInfo> Accept();

 private:
  void PrepareListeningFd(size_t port, AddressFamily af);

 private:
  io::Fd lfd_;
};

}  // namespace harmony::net
