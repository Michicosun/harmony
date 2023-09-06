#pragma once

namespace harmony::io {

using Fd = int;

void MakeNonblocking(Fd fd);

}  // namespace harmony::io
