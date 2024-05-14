//
// Created by Jorge on 08/05/2024.
//

#include <filesystem>
#include <iostream>

#include <boost/core/make_span.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

#include "files/graph.h"
#include "files/tree.h"
#include "files/helper.h"

static auto const resources_dir =
    std::filesystem::path(TEST_RESOURCES_DIR);  // NOLINT(cert-err58-cpp)

using namespace album_architect;
using namespace std::string_literals;
namespace fs = std::filesystem;

TEST_CASE("Tree structure of directory", "[files][tree]") {
  // Check that passing a file returns None
  auto invalid_tree =
      files::FileTree::build(resources_dir / "album_one" / "one.1.jpg");
  REQUIRE_FALSE(invalid_tree);

  // Create a directory tree from the test tree
  auto directory_tree = files::FileTree::build(resources_dir);
  REQUIRE(directory_tree);

  // Check that some paths are available
  const auto not_valid_path = std::filesystem::path("/not/a/valid/path");
  REQUIRE_FALSE(directory_tree->contains_path(not_valid_path));

  const auto album_one_path = resources_dir / "album_one";
  REQUIRE(directory_tree->contains_path(album_one_path)
          == files::PathType::directory);

  const auto album_three_one_1_path =
      resources_dir / "album_three" / "album_three_one" / "three.one.1.txt";
  REQUIRE(directory_tree->contains_path(album_three_one_1_path)
          == files::PathType::file);
}

TEST_CASE("Graph for directories", "[files][graph]") {
  auto tree_graph = files::FileGraph {};

  // Get root node
  auto root_node_type = tree_graph.get_node_type({});
  REQUIRE(root_node_type == files::NodeType::directory);

  // Get another node
  auto path1 = std::vector {"route"s, "to"s, "first"s};
  auto path1_node_type = tree_graph.get_node_type(path1);
  REQUIRE_FALSE(path1_node_type);

  tree_graph.add_node(path1, files::NodeType::file);
  path1_node_type = tree_graph.get_node_type(path1);
  REQUIRE(path1_node_type == files::NodeType::file);

  while (!path1.empty()) {
    path1.pop_back();
    REQUIRE(tree_graph.get_node_type(path1) == files::NodeType::directory);
  }

  auto path2 = std::vector{"route"s, "to"s, "second"s};
  REQUIRE_FALSE(tree_graph.get_node_type(path2));

  tree_graph.add_node(path2, files::NodeType::file);
  REQUIRE(tree_graph.get_node_type(path2) == files::NodeType::file);

  // Try to rename a node
  path1 = std::vector {"route"s, "to"s, "first"s};
  auto new_path1 = std::vector{"route"s, "to"s, "new_first"s};
  tree_graph.rename_node(path1, "new_first"s);
  REQUIRE_FALSE(tree_graph.get_node_type(path1));
  REQUIRE(tree_graph.get_node_type(new_path1) == files::NodeType::file);

  // Try to rename a node that has children
  auto subpath = std::vector{"route"s, "to"s};
  tree_graph.rename_node(subpath, "new_to");
  REQUIRE_FALSE(tree_graph.get_node_type(path2));

  path2[1] = "new_to";
  REQUIRE(tree_graph.get_node_type(path2) == files::NodeType::file);

  // ... renaming the root should fail
  REQUIRE_FALSE(tree_graph.rename_node({}, "new_root_name"));

  // DEBUG: Show graph representation
  tree_graph.to_graphviz(std::cout);
}

TEST_CASE("TempCurrentDir tests", "[files][helper]" ){
  // Get current directories
  auto original_dir = fs::current_path();

  auto temporary_dir = fs::temp_directory_path().parent_path();
  REQUIRE(original_dir != temporary_dir);

  // Create temporary dir
  {
    auto temp = files::TempCurrentDir(temporary_dir);
    REQUIRE(fs::current_path() == temporary_dir);
    REQUIRE(fs::current_path() != original_dir);
  }

  // Check original dir
  REQUIRE(fs::current_path() != temporary_dir);
  REQUIRE(fs::current_path() == original_dir);
}