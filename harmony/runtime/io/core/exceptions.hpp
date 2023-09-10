#pragma once

#include <stdexcept>

namespace harmony::io {

class IOError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class EpollError : IOError {
  using IOError::IOError;
};

class EventFdError : IOError {
  using IOError::IOError;
};

}  // namespace harmony::io
