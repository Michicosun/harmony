#pragma once

#include <stdexcept>

namespace harmony::net {

class NetError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

}  // namespace harmony::net
