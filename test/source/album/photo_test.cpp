#include <iostream>

#include <album/photo.h>
#include <catch2/catch_template_test_macros.hpp>

using album_architect::Photo;
TEST_CASE("Load a photo", "[album][photo]") {
  auto invalid_photo = Photo::load("not_a_path.jpeg");
  REQUIRE_FALSE(invalid_photo);

  std::cout << std::filesystem::current_path() << '\n';
}