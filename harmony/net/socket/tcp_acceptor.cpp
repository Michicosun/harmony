#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include <harmony/coro/events/fd_ready.hpp>
#include <harmony/net/exceptions.hpp>
#include <harmony/net/socket/tcp_acceptor.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

TcpAcceptor::TcpAcceptor(size_t port, AddressFamily af) {
  PrepareListeningFd(port, af);
}

TcpAcceptor::~TcpAcceptor() {
  close(lfd_);
}

coro::Task<AcceptInfo> TcpAcceptor::Accept() {
  struct sockaddr_storage ss;
  socklen_t addrlen = sizeof(ss);

  auto status = co_await coro::FdReady(lfd_, io::Operation::Read);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  if (status == io::EventStatus::Closed) {
    throw NetError("descriptor is closed");
  }

  int client_fd = accept(lfd_, (struct sockaddr*)&ss, &addrlen);

  if (client_fd < 0) {
    throw NetError(strerror(errno));
  }

  io::MakeNonblocking(client_fd);

  switch (ss.ss_family) {
    case AF_INET: {
      struct sockaddr_in* addr = (struct sockaddr_in*)&ss;

      char ip_address_buffer[INET_ADDRSTRLEN] = {0};
      auto* ptr = inet_ntop(AF_INET, &addr->sin_addr, ip_address_buffer,
                            INET_ADDRSTRLEN);

      if (ptr == nullptr) {
        throw NetError(strerror(errno));
      }

      co_return AcceptInfo{
          .fd = client_fd,
          .ip_address = ip_address_buffer,
          .port = addr->sin_port,
      };
    }
    case AF_INET6: {
      struct sockaddr_in6* addr = (struct sockaddr_in6*)&ss;

      char ip_address_buffer[INET6_ADDRSTRLEN] = {0};
      auto* ptr = inet_ntop(AF_INET6, &addr->sin6_addr, ip_address_buffer,
                            INET6_ADDRSTRLEN);

      if (ptr == nullptr) {
        throw NetError(strerror(errno));
      }

      co_return AcceptInfo{
          .fd = client_fd,
          .ip_address = ip_address_buffer,
          .port = addr->sin6_port,
      };
    }
  }

  throw NetError("incorrect address family");
}

void TcpAcceptor::PrepareListeningFd(size_t port, AddressFamily af) {
  lfd_ = socket(AddressFamilyToNative(af), SOCK_STREAM, 0);
  if (lfd_ < 0) {
    throw NetError(strerror(errno));
  }

  const int enable = 1;
  if (setsockopt(lfd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable,
                 sizeof(int)) < 0) {
    throw NetError(strerror(errno));
  }

  struct sockaddr_storage ss = {};
  socklen_t addrlen;

  int family = AddressFamilyToNative(af);
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

  if (bind(lfd_, (struct sockaddr*)&ss, addrlen) < 0) {
    throw NetError(strerror(errno));
  }

  if (listen(lfd_, 5) < 0) {
    throw NetError(strerror(errno));
  }

  io::MakeNonblocking(lfd_);
}

}  // namespace harmony::net
