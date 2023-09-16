#pragma once

#include <optional>

#include <harmony/coro/core/task.hpp>
#include <harmony/net/buffer.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

class TcpSocket {
 public:
  explicit TcpSocket(AddressFamily family);
  explicit TcpSocket(io::Fd con_fd);

  ~TcpSocket();

 public:
  coro::Task<> Connect(std::string host, size_t port);

  template <class... Args>
  coro::Task<size_t> ReadSome(Args&&... args) {
    return ReadSome(Buffer(std::forward<Args>(args)...));
  }

  template <class... Args>
  coro::Task<size_t> WriteSome(Args&&... args) {
    return WriteSome(Buffer(std::forward<Args>(args)...));
  }

 private:
  coro::Task<size_t> ReadSome(Buffer buffer);
  coro::Task<size_t> WriteSome(Buffer buffer);

 private:
  std::optional<AddressFamily> family_{AddressFamily::IPv4};
  io::Fd con_fd_{-1};
};

}  // namespace harmony::net
