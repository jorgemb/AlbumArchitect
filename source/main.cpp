#include <fstream>
#include <utility>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "files/tree.h"

namespace fs = std::filesystem;

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
  auto file_tree = album_architect::files::FileTree::build(path);
  if (!file_tree) {
    spdlog::error("Couldn't create a tree of path: {}", path.string());
    return -1;
  }

  // .. serialize
  fmt::println("Serializing");
  auto serialize_output = std::ofstream("filesystem.data");
  if (serialize_output) {
    file_tree->to_stream(serialize_output);
  }

  // ... write to graphviz
  //  fmt::println("Writing to Graphviz");
  //  auto output_file = std::ofstream("filesystem.dot");
  //  if (output_file) {
  //    file_tree->to_graphviz(output_file);
  //  }

  return 0;
}
