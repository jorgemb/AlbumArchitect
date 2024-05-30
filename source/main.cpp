#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "files/tree.h"

namespace fs = std::filesystem;

auto load_or_create_filetree(const fs::path& directory,
                             const fs::path& saved_state)
    -> std::optional<album_architect::files::FileTree> {
  auto file_tree = std::optional<album_architect::files::FileTree> {};
  if (exists(saved_state)) {
    // Try loading from filesystem
    spdlog::info("Trying to load from path {}", saved_state.string());
    auto file_data = std::ifstream(saved_state);
    file_tree = album_architect::files::FileTree::from_stream(file_data);
    if (!file_tree) {
      spdlog::error("Couldn't load from path");
      return {};
    }
  } else {
    // Calculate filetree
    spdlog::info("Calculating file tree");
    file_tree = album_architect::files::FileTree::build(directory);
    if (!file_tree) {
      spdlog::error("Couldn't create a tree of path: {}", directory.string());
      return {};
    }
  }
}

auto main(int argc, char* argv[]) -> int {
  // Check usage
  if (argc != 2) {
    fmt::println("Usage: albumarchitect <FOLDER>");
    return -1;
  }

  // Try to create path
  auto path = fs::path(argv[1]);
  if (!fs::exists(path) || !fs::is_directory(path)) {
    spdlog::error("The given path {} is not valid.", path.string());
    return -1;
  }

  // Create the file tree and output
  const auto output_path = fs::path("filesystem.data");
  auto file_tree = load_or_create_filetree(path, output_path);

  // Output all the directories
  auto directory_stack =
      std::deque<std::pair<std::uint32_t, album_architect::files::Element>> {};
  directory_stack.emplace_back(0U, file_tree->get_root_element());
  while (!directory_stack.empty()) {
    // Get current element
    auto [level, current_element] = std::move(directory_stack.front());
    directory_stack.pop_front();

    // Print current element
    for (auto _ = 0U; _ != level; ++_) {
      std::cout << '|';
    }
    std::cout << current_element.get_path().filename() << '\n';

    // Add all the directories from the current element
    for (auto&& child : current_element.get_children()) {
      if (child.get_type() == album_architect::files::PathType::directory) {
        directory_stack.emplace_front(level + 1, std::move(child));
      }
    }
  }

  // .. serialize
  spdlog::info("SERIALIZING");
  auto serialize_output = std::ofstream(output_path);
  if (serialize_output) {
    file_tree->to_stream(serialize_output);
  }

  // ... write to graphviz
  //  fmt::println("Writing to Graphviz");
  //  auto output_file = std::ofstream("filesystem.dot");
  //  if (output_file) {
  //    file_tree->to_graphviz(output_file);
  //  }

  spdlog::info("DESTROYING");

  return 0;
}
