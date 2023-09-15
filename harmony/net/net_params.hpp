#pragma once

#include <string>

namespace harmony::net {

enum class AddressFamily {
  Unknown,
  IPv4,
  IPv6,
};

struct ConnectionParams {
  std::string ip_address;
  AddressFamily address_family;
};

int AddressFamilyToNative(AddressFamily af);
AddressFamily AddressFamilyFromNative(int native);

}  // namespace harmony::net
