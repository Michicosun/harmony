#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <harmony/runtime/io/core/exceptions.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::io {

void MakeNonblocking(Fd fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    throw FDError(strerror(errno));
  }

  int code = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (code < 0) {
    throw FDError(strerror(errno));
  }
}

}  // namespace harmony::io
