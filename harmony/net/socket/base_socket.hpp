#pragma once

#include <harmony/coro/core/task.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

class BaseSocket {
 protected:
  BaseSocket() = default;
  explicit BaseSocket(io::Fd con_fd);

  ~BaseSocket();

  enum class SocketType {
    Tcp,
    Udp,
  };

 protected:
  coro::Task<> Connect(SocketType type, ConnectionParams params, size_t port);

  static int SocketTypeToNative(SocketType type);

 protected:
  std::optional<io::Fd> con_fd_;
};

};  // namespace harmony::net
