//
// Created by Jorge on 08/05/2024.
//

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <thread>
#include <vector>

#include <boost/core/make_span.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
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

  SECTION("Basic path discovery and checks") {
    // Check that some paths are available
    const auto not_valid_path = std::filesystem::path("/not/a/valid/path");
    REQUIRE_FALSE(directory_tree->get_element(not_valid_path));

    const auto album_one_path = resources_dir / "album_one";
    const auto album_one_element = directory_tree->get_element(album_one_path);
    REQUIRE(album_one_element);
    REQUIRE(album_one_element->get_path() == album_one_path);
    REQUIRE(album_one_element->get_type() == files::PathType::directory);

    const auto album_three_one_1_path =
        resources_dir / "album_three" / "album_three_one" / "three.one.1.txt";
    const auto album_three_one_1_element =
        directory_tree->get_element(album_three_one_1_path);
    REQUIRE(album_three_one_1_element);
    REQUIRE(album_three_one_1_element->get_path() == album_three_one_1_path);
    REQUIRE(album_three_one_1_element->get_type() == files::PathType::file);
  }

  SECTION("Serialization") {
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

  auto expected_root_children = std::vector {resources_dir / "album_one",
                                             resources_dir / "album_two",
                                             resources_dir / "album_three"};

  SECTION("Siblings, children and parent") {
    auto children = std::vector<files::Element> {};
    REQUIRE_FALSE(directory_tree->get_elements_under_path(
        fs::path("/this/is/an/invalid/path"), children));

    REQUIRE(directory_tree->get_elements_under_path(resources_dir, children));

    auto children_paths = std::vector<fs::path> {};
    std::transform(children.begin(),
                   children.end(),
                   std::back_inserter(children_paths),
                   [](auto element) { return element.get_path(); });
    REQUIRE_THAT(expected_root_children,
                 Catch::Matchers::UnorderedRangeEquals(children_paths));
  }

  SECTION("Element management") {
    auto root_element = directory_tree->get_element({});
    REQUIRE(root_element);
    REQUIRE(root_element->get_path() == resources_dir);
    REQUIRE(root_element->get_type()
            == album_architect::files::PathType::directory);

    // .. root doesn't have siblings
    REQUIRE_FALSE(root_element->get_parent());
    REQUIRE(root_element->get_siblings().empty());

    auto root_children_elements = root_element->get_children();
    auto root_children = decltype(expected_root_children) {};
    std::transform(root_children_elements.begin(),
                   root_children_elements.end(),
                   std::back_inserter(root_children),
                   [](auto elem) { return elem.get_path(); });

    REQUIRE_THAT(expected_root_children,
                 Catch::Matchers::UnorderedRangeEquals(root_children));
  }
}

TEST_CASE("Graph for directories", "[files][graph]") {
  auto tree_graph = files::FileGraph {/*create_root=*/true};

  // Get root node_id
  auto root_node_type = tree_graph.get_node_type({});
  REQUIRE(root_node_type == files::NodeType::directory);

  // Add the paths
  auto path1 = std::vector {"route"s, "to"s, "first"s};
  tree_graph.add_node(path1, files::NodeType::file);
  auto path2 = std::vector {"route"s, "to"s, "second"s};
  tree_graph.add_node(path2, files::NodeType::file);

  SECTION("Basic retrieval of nodes") {
    const auto non_existent_path =
        std::vector {"route"s, "to"s, "non"s, "existant"s};
    REQUIRE_FALSE(tree_graph.get_node(non_existent_path));

    // Get another type
    auto path1_node = tree_graph.get_node(path1);
    REQUIRE(path1_node);

    auto path1_node_type = tree_graph.get_node_type(*path1_node);
    REQUIRE(path1_node_type == files::NodeType::file);

    while (!path1.empty()) {
      path1.pop_back();
      auto subpath_node = tree_graph.get_node(path1);
      REQUIRE(subpath_node);
      REQUIRE(tree_graph.get_node_type(*subpath_node)
              == files::NodeType::directory);
    }

    // Get children of a node
    auto root_node = tree_graph.get_node({});
    REQUIRE(root_node);
    REQUIRE(tree_graph.get_node_type(*root_node) == files::NodeType::directory);

    auto root_children = tree_graph.get_node_children(*root_node);
    REQUIRE(root_children.size() == 1);

    // ... add new path to root and check children
    auto path3 = std::vector {"path"s, "to"s, "third"s};
    tree_graph.add_node(path3, files::NodeType::file);

    root_children = tree_graph.get_node_children(*root_node);
    REQUIRE(root_children.size() == 2);

    auto path3_node = tree_graph.get_node(path3);
    REQUIRE(path3_node);
    auto path3_node_path = tree_graph.get_node_path(*path3_node);
    REQUIRE(path3 == path3_node_path);
  }

  SECTION("Renaming nodes") {
    // Try to rename a node_id
    auto new_path1 = std::vector {"route"s, "to"s, "new_first"s};
    tree_graph.rename_node(path1, "new_first"s);
    REQUIRE_FALSE(tree_graph.get_node(path1));

    auto new_path1_node = tree_graph.get_node(new_path1);
    REQUIRE(new_path1_node);
    REQUIRE(tree_graph.get_node_type(*new_path1_node) == files::NodeType::file);

    // Try to rename a node_id that has children
    auto subpath = std::vector {"route"s, "to"s};
    tree_graph.rename_node(subpath, "new_to");
    REQUIRE_FALSE(tree_graph.get_node(path2));

    path2[1] = "new_to";
    auto new_path2_node = tree_graph.get_node(path2);
    REQUIRE(new_path2_node);
    REQUIRE(tree_graph.get_node_type(*new_path2_node) == files::NodeType::file);

    // ... renaming the root should fail
    REQUIRE_FALSE(tree_graph.rename_node({}, "new_root_name"));
  }

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