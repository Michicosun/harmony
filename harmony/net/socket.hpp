#pragma once

#include <span>

#include <harmony/coro/core/task.hpp>
#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

class TcpSocket {
  struct Result {
    io::EventStatus status;
    size_t bytes_transferred;
  };

 public:
  explicit TcpSocket(io::Fd con_fd);

  ~TcpSocket();

  coro::Task<Result> AsyncReadSome(std::span<std::byte> buffer);
  coro::Task<Result> AsyncWriteSome(std::span<std::byte> buffer);

 private:
  io::Fd con_fd_;
};

}  // namespace harmony::net
