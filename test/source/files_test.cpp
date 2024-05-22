//
// Created by Jorge on 08/05/2024.
//

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <boost/core/make_span.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

#include "files/graph.h"
#include "files/helper.h"
#include "files/tree.h"

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
  const auto album_one_element = directory_tree->contains_path(album_one_path);
  REQUIRE(album_one_element);
  REQUIRE(album_one_element->get_path() == album_one_path);
  REQUIRE(album_one_element->get_type() == files::PathType::directory);

  const auto album_three_one_1_path =
      resources_dir / "album_three" / "album_three_one" / "three.one.1.txt";
  const auto album_three_one_1_element =
      directory_tree->contains_path(album_three_one_1_path);
  REQUIRE(album_three_one_1_element);
  REQUIRE(album_three_one_1_element->get_path() == album_three_one_1_path);
  REQUIRE(album_three_one_1_element->get_type() == files::PathType::file);

  // Check that the tree can be serialized
  auto temp_file = files::TemporaryFile {};
  {
    auto out_file = std::ofstream {temp_file.get_path()};
    directory_tree->to_stream(out_file);
  }

  // Try to get the tree and compare
  auto in_file = std::ifstream {temp_file.get_path()};
  auto new_tree = files::FileTree::from_stream(in_file);
  REQUIRE(new_tree);
  REQUIRE(directory_tree == new_tree);
}

TEST_CASE("Graph for directories", "[files][graph]") {
  auto tree_graph = files::FileGraph {true};

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

  auto path2 = std::vector {"route"s, "to"s, "second"s};
  REQUIRE_FALSE(tree_graph.get_node_type(path2));

  tree_graph.add_node(path2, files::NodeType::file);
  REQUIRE(tree_graph.get_node_type(path2) == files::NodeType::file);

  // Try to rename a node
  path1 = std::vector {"route"s, "to"s, "first"s};
  auto new_path1 = std::vector {"route"s, "to"s, "new_first"s};
  tree_graph.rename_node(path1, "new_first"s);
  REQUIRE_FALSE(tree_graph.get_node_type(path1));
  REQUIRE(tree_graph.get_node_type(new_path1) == files::NodeType::file);

  // Try to rename a node that has children
  auto subpath = std::vector {"route"s, "to"s};
  tree_graph.rename_node(subpath, "new_to");
  REQUIRE_FALSE(tree_graph.get_node_type(path2));

  path2[1] = "new_to";
  REQUIRE(tree_graph.get_node_type(path2) == files::NodeType::file);

  // ... renaming the root should fail
  REQUIRE_FALSE(tree_graph.rename_node({}, "new_root_name"));

  // DEBUG: Show graph representation
  //  tree_graph.to_graphviz(std::cout);
}

TEST_CASE("TempCurrentDir tests", "[files][helper]") {
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

TEST_CASE("TemporaryFile tests", "[files][helper]") {
  auto temporary_file = std::make_unique<files::TemporaryFile>();
  auto path = temporary_file->get_path();
  REQUIRE_FALSE(fs::exists(path));

  {
    // Write to file to make it exist
    auto file = std::ofstream(path);
    file << "This is a test";
  }

  REQUIRE(fs::exists(path));

  // Sleep to allow for the file handler to be released
  std::this_thread::sleep_for(std::chrono::seconds(1));
  temporary_file.reset();
  REQUIRE_FALSE(fs::exists(path));
}