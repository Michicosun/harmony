#include <iostream>

#include <harmony/support/terminate/terminate.hpp>

namespace harmony::support {

void Terminate(const std::string& error) {
  std::cerr << "HARMONY FATAL ERROR: " + error << std::endl;
  std::terminate();
}

}  // namespace harmony::support
