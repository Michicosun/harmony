#include <fcntl.h>
#include <unistd.h>

#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::io {

void MakeNonblocking(Fd fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

}  // namespace harmony::io
