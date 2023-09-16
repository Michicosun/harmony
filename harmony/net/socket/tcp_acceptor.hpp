#pragma once

#include <harmony/coro/core/task.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

struct AcceptInfo {
  AddressInfo con_info;
  io::Fd fd;
};

class TcpAcceptor {
 public:
  explicit TcpAcceptor(size_t port, AddressFamily af = AddressFamily::IPv4);

  ~TcpAcceptor();

  coro::Task<AcceptInfo> Accept();

 private:
  void PrepareListeningFd(size_t port, AddressFamily af);

 private:
  io::Fd lfd_;
};

}  // namespace harmony::net
