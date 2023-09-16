#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>

#include <harmony/net/exceptions.hpp>
#include <harmony/net/net_params.hpp>

namespace harmony::net {

AddressInfo AddressInfoFromStorage(struct sockaddr_storage storage) {
  switch (storage.ss_family) {
    case AF_INET: {
      struct sockaddr_in* addr = (struct sockaddr_in*)&storage;

      char ip_address_buffer[INET_ADDRSTRLEN] = {0};
      auto* ptr = inet_ntop(AF_INET, &addr->sin_addr, ip_address_buffer,
                            INET_ADDRSTRLEN);

      if (ptr == nullptr) {
        throw NetError(strerror(errno));
      }

      return {
          .address_family = AddressFamily::IPv4,
          .ip_address = ip_address_buffer,
          .port = ntohs(addr->sin_port),
      };
    }
    case AF_INET6: {
      struct sockaddr_in6* addr = (struct sockaddr_in6*)&storage;

      char ip_address_buffer[INET6_ADDRSTRLEN] = {0};
      auto* ptr = inet_ntop(AF_INET6, &addr->sin6_addr, ip_address_buffer,
                            INET6_ADDRSTRLEN);

      if (ptr == nullptr) {
        throw NetError(strerror(errno));
      }

      return {
          .address_family = AddressFamily::IPv6,
          .ip_address = ip_address_buffer,
          .port = ntohs(addr->sin6_port),
      };
    }
  }

  throw NetError("incorrect address family");
}

int AddressFamilyToNative(AddressFamily af) {
  switch (af) {
    case AddressFamily::IPv4:
      return AF_INET;
    case AddressFamily::IPv6:
      return AF_INET6;
    case AddressFamily::Unknown:
      return AF_UNSPEC;
  }
}

AddressFamily AddressFamilyFromNative(int native) {
  if (native == AF_INET) {
    return AddressFamily::IPv4;
  } else if (native == AF_INET6) {
    return AddressFamily::IPv6;
  } else {
    return AddressFamily::Unknown;
  }
}

}  // namespace harmony::net
