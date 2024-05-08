//
// Created by Jorge on 08/05/2024.
//

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fmt/core.h>
#include "files/tree.h"

static auto const resources_dir = std::filesystem::path(TEST_RESOURCES_DIR); // NOLINT(cert-err58-cpp)

using namespace album_architect;

TEST_CASE("Tree structure of directory", "[files][tree]"){
  // Check that passing a file returns None
  auto invalid_tree = files::FileTree::create_file_tree(resources_dir / "album_one" / "one.1.jpg");
  REQUIRE_FALSE(invalid_tree);

  // Create a directory tree from the test tree
  auto directory_tree = files::FileTree::create_file_tree(resources_dir);
  REQUIRE(directory_tree);

  // Check that some paths are available
  const auto not_valid_path = std::filesystem::path("not/a/valid/path");
  REQUIRE_FALSE(directory_tree->contains_path(not_valid_path));

  const auto album_one_path = resources_dir / "album_one";
  REQUIRE(directory_tree->contains_path(album_one_path) == files::PathType::directory);

  const auto album_three_one_1_path = resources_dir / "album_three" / "album_three_one" / "three.one.1.txt";
  REQUIRE(directory_tree->contains_path(album_three_one_1_path) == files::PathType::file);
}