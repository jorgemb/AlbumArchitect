#include <filesystem>

#include <album/util.h>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glog/logging.h>

#include "config/config.h"
#include "lib.hpp"

namespace fs = std::filesystem;

TEST_CASE("Name is AlbumArchitect", "[library]") {
  auto const lib = library {};
  REQUIRE(lib.name == "AlbumArchitect");
}

TEST_CASE("Temporary directory creation", "[utility]") {
  auto temp_path = std::filesystem::path {};
  {
    const auto temp_directory = album_architect::util::AutoTempDirectory {};
    temp_path = temp_directory.path();

    REQUIRE(fs::exists(temp_path));
    REQUIRE(fs::is_directory(temp_path));

    REQUIRE(temp_path.filename().string() == temp_directory.name());
  }

  // On deletion the path should not be usable anymore
  REQUIRE_FALSE(fs::exists(temp_path));
}

auto main(int argc, char* argv[]) -> int {
  google::InitGoogleLogging(
      argv[0]);  // NOLINT(*-pro-bounds-pointer-arithmetic)

  // Load config data
  album_architect::Config::load("config.test.yaml");

  auto result = Catch::Session().run(argc, argv);

  return result;
}