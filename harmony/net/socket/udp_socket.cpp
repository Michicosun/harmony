#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include <harmony/coro/events/fd_ready.hpp>
#include <harmony/net/exceptions.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/net/socket/udp_socket.hpp>

namespace harmony::net {

UdpSocket::UdpSocket(AddressFamily family)
    : family_{family} {
  con_fd_ = socket(AddressFamilyToNative(family), SOCK_DGRAM, 0);
  if (con_fd_ < 0) {
    throw NetError(strerror(errno));
  }

  io::MakeNonblocking(con_fd_);
}

UdpSocket::~UdpSocket() {
  close(con_fd_);
}

void UdpSocket::Bind(size_t port) {
  struct sockaddr_storage ss = {};
  socklen_t addrlen;

  int family = AddressFamilyToNative(family_);
  switch (family) {
    case AF_INET: {
      struct sockaddr_in* addr = (struct sockaddr_in*)&ss;
      addr->sin_family = AF_INET;
      addr->sin_port = htons(port);
      addr->sin_addr.s_addr = INADDR_ANY;
      addrlen = sizeof(struct sockaddr_in);
      break;
    }

    case AF_INET6: {
      struct sockaddr_in6* addr = (struct sockaddr_in6*)&ss;
      addr->sin6_family = AF_INET6;
      addr->sin6_port = htons(port);
      addr->sin6_addr = in6addr_any;
      addrlen = sizeof(struct sockaddr_in6);
      break;
    }
  }

  // bind socket to port
  if (bind(con_fd_, (struct sockaddr*)&ss, addrlen) < 0) {
    throw NetError(strerror(errno));
  }
}

coro::Task<DatagramInfo> UdpSocket::Receive(Buffer buffer) {
  auto status = co_await coro::FdReady(con_fd_, io::Operation::Read);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  struct sockaddr_storage ss;
  socklen_t addrlen = sizeof(ss);

  int64_t n = recvfrom(con_fd_, buffer.Data(), buffer.Size(), MSG_DONTWAIT,
                       (struct sockaddr*)&ss, &addrlen);

  if (n < 0) {
    throw NetError(strerror(errno));
  }

  co_return DatagramInfo{
      .sender = AddressInfoFromStorage(std::move(ss)),
      .size = static_cast<size_t>(n),
  };
}

coro::Task<size_t> UdpSocket::Send(AddressInfo receiver, Buffer buffer) {
  auto status = co_await coro::FdReady(con_fd_, io::Operation::Write);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  struct sockaddr_storage ss;
  socklen_t addrlen;

  switch (receiver.address_family) {
    case AddressFamily::IPv4: {
      struct sockaddr_in* addr = (struct sockaddr_in*)&ss;
      addr->sin_family = AF_INET;
      addr->sin_port = htons(receiver.port);
      inet_pton(AF_INET, receiver.ip_address.c_str(), &addr->sin_addr);
      addrlen = sizeof(struct sockaddr_in);
      break;
    }
    case AddressFamily::IPv6: {
      struct sockaddr_in6* addr = (struct sockaddr_in6*)&ss;
      addr->sin6_family = AF_INET6;
      addr->sin6_port = htons(receiver.port);
      inet_pton(AF_INET6, receiver.ip_address.c_str(), &addr->sin6_addr);
      addrlen = sizeof(struct sockaddr_in6);
      break;
    }
    case AddressFamily::Unknown: {
      throw NetError("undefined address family of receiver");
    }
  }

  int64_t n = sendto(con_fd_, buffer.Data(), buffer.Size(), MSG_DONTWAIT,
                     (struct sockaddr*)&ss, addrlen);

  if (n < 0) {
    throw NetError(strerror(errno));
  }

  co_return n;
}

}  // namespace harmony::net
