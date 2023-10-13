#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glog/logging.h>

#include "lib.hpp"

TEST_CASE("Name is AlbumArchitect", "[library]") {
  auto const lib = library {};
  REQUIRE(lib.name == "AlbumArchitect");
}

auto main(int argc, char* argv[]) -> int {
  google::InitGoogleLogging(argv[0]); // NOLINT(*-pro-bounds-pointer-arithmetic)

  auto result = Catch::Session().run(argc, argv);

  return result;
}