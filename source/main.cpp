#include <algorithm>
#include <deque>
#include <execution>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "album/photo.h"
#include "album/photo_metadata.h"
#include "files/tree.h"

namespace fs = std::filesystem;
namespace rng = std::ranges;
namespace vws = std::ranges::views;
using album_architect::album::ImageHashAlgorithm;
using album_architect::album::PhotoMetadata;

auto main(int argc, char* argv[]) -> int {
  // Check usage
  if (argc != 2) {
    fmt::println("Usage: album_architect <FOLDER>");
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
  auto file_tree = std::optional<album_architect::files::FileTree> {};
  if (exists(output_path)) {
    // Try loading from filesystem
    spdlog::info("Trying to load from path {}", output_path.string());
    auto file_data = std::ifstream(output_path);
    file_tree = album_architect::files::FileTree::from_stream(file_data);
    if (!file_tree) {
      return -1;
    }
  } else {
    // Calculate filetree
    spdlog::info("Calculating file tree");
    file_tree = album_architect::files::FileTree::build(path);
    if (!file_tree) {
      spdlog::error("Couldn't create a tree of path: {}", path.string());
      return -1;
    }
  }

  // Get all elements
  auto is_file = [](const album_architect::files::Element& element)
  { return element.get_type() == album_architect::files::PathType::file; };
  auto file_elements = std::vector<album_architect::files::Element> {};
  std::copy_if(file_tree->begin(),
               file_tree->end(),
               std::back_inserter(file_elements),
               is_file);

  // Load all hashes
  spdlog::info("LOADING HASHES");
  std::for_each(std::execution::par_unseq,
                file_elements.begin(),
                file_elements.end(),
                [](auto& element)
                {
                  if (auto photo = album_architect::album::Photo::load(element))
                  {
                    photo->get_image_hash(ImageHashAlgorithm::average_hash);
                    photo->get_image_hash(ImageHashAlgorithm::p_hash);
                  }
                });

  // .. serialize
  spdlog::info("SERIALIZING");
  {
    if (auto serialize_output = std::ofstream(output_path)) {
      file_tree->to_stream(serialize_output);
    }
  }

  // ... write to graphviz
  fmt::println("Writing to Graphviz");
  if (auto output_file = std::ofstream("filesystem.dot")) {
    file_tree->to_graphviz(output_file);
  }

  spdlog::info("DESTROYING");

  return 0;
}
