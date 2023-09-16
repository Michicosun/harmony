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
  coro::Task<size_t> AsyncReadSome(Args&&... args) {
    return AsyncReadSome(net::Buffer(std::forward<Args>(args)...));
  }

  template <class... Args>
  coro::Task<size_t> AsyncWriteSome(Args&&... args) {
    return AsyncWriteSome(net::Buffer(std::forward<Args>(args)...));
  }

 private:
  coro::Task<size_t> AsyncReadSome(Buffer buffer);
  coro::Task<size_t> AsyncWriteSome(Buffer buffer);

 private:
  std::optional<AddressFamily> family_{AddressFamily::IPv4};
  io::Fd con_fd_{-1};
};

}  // namespace harmony::net
