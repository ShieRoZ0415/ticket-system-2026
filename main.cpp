#include <iosfwd>
#include <iostream>

#include "cmd.h"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  Sys sys;
  sys.run();

  return 0;
}