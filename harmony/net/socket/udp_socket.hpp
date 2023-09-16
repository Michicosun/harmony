#pragma once

#include <harmony/coro/core/task.hpp>
#include <harmony/net/buffer.hpp>
#include "harmony/net/net_params.hpp"
#include "harmony/runtime/io/core/fd.hpp"

namespace harmony::net {

struct DatagramInfo {
  AddressInfo sender;
  size_t size;
};

class UdpSocket {
 public:
  explicit UdpSocket(AddressFamily family);

  ~UdpSocket();

 public:
  void Bind(size_t port);

  template <class... Args>
  coro::Task<DatagramInfo> AsyncReceive(Args&&... args) {
    return AsyncReceive(Buffer(std::forward<Args>(args)...));
  }

  template <class... Args>
  coro::Task<size_t> AsyncSend(AddressInfo receiver, Args&&... args) {
    return AsyncSend(std::move(receiver), Buffer(std::forward<Args>(args)...));
  }

 private:
  coro::Task<DatagramInfo> AsyncReceive(Buffer buffer);
  coro::Task<size_t> AsyncSend(AddressInfo receiver, Buffer buffer);

 private:
  AddressFamily family_{AddressFamily::IPv4};
  io::Fd con_fd_{-1};
};

}  // namespace harmony::net
