#pragma once

#include <sys/socket.h>

#include <cstdint>
#include <string>

namespace harmony::net {

enum class AddressFamily {
  Unknown,
  IPv4,
  IPv6,
};

struct AddressInfo {
  AddressFamily address_family;
  std::string ip_address;
  uint16_t port;
};

AddressInfo AddressInfoFromStorage(struct sockaddr_storage storage);

int AddressFamilyToNative(AddressFamily af);
AddressFamily AddressFamilyFromNative(int native);

}  // namespace harmony::net
