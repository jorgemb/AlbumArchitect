#include <cstdint>
#include <deque>
#include <execution>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "album/photo.h"
#include "album/photo_metadata.h"
#include "files/tree.h"

namespace fs = std::filesystem;
using album_architect::album::ImageHashAlgorithm;

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
    spdlog::info("Trying to load from path {}");
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
  auto directory_stack =
      std::deque<std::pair<std::uint32_t, album_architect::files::Element>> {};
  auto files_to_load = std::vector<album_architect::files::Element> {};
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

    // Load photo and hash of current element
    if (current_element.get_type() == album_architect::files::PathType::file) {
      files_to_load.push_back(std::move(current_element));
    }

    // Add all the directories from the current element
    for (auto&& child : current_element.get_children()) {
      directory_stack.emplace_front(level + 1, std::move(child));
    }
  }

  // Load all hashes
  spdlog::info("LOADING HASHES");
  std::for_each(
      std::execution::par_unseq,
      files_to_load.begin(),
      files_to_load.end(),
      [](auto& element)
      {
        if (album_architect::album::PhotoMetadata::get_photo_state(element)
            != album_architect::album::PhotoState::error)
        {
          if (auto photo = album_architect::album::Photo::load(element)) {
            try {
              photo->get_image_hash(ImageHashAlgorithm::average_hash);
              photo->get_image_hash(ImageHashAlgorithm::p_hash);
            }catch(const cv::Exception& e) {
              spdlog::error("Failed to hash photo: {}. Error: {}", element.get_path().string(), e.what());
              album_architect::album::PhotoMetadata::set_photo_state(element, album_architect::album::PhotoState::error);
            }
          }
        }
      });

  // .. serialize
  spdlog::info("SERIALIZING");
  {
    auto serialize_output = std::ofstream(output_path);
    if (serialize_output) {
      file_tree->to_stream(serialize_output);
    }
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
