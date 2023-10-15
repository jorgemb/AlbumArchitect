#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <album/photo.h>

#include "lib.hpp"

using namespace std::string_literals;

auto main() -> int {
  auto file = "test/sample-images/Samples/HEIC/IMG_0378.HEIC"s;
  auto photo = album_architect::Photo::load(file);

  return 0;
}
