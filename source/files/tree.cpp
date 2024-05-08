//
// Created by Jorge on 08/05/2024.
//

#include <exception>
#include <filesystem>
#include <utility>

#include "tree.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace album_architect::files {

auto FileTree::create_file_tree(const std::filesystem::path& path)
    -> std::optional<FileTree> {
  // Check if path is directory or file
  if (!fs::is_directory(path)) {
    return {};
  }

  return std::make_optional<FileTree>(FileTree {path});
}
auto FileTree::add_directory(const std::filesystem::path& path,
                             bool add_files,
                             bool recursive) -> bool {
  // Check that directory is subdirectory of root
  auto error_code = std::error_code{};
  auto relative_path = fs::relative(path, m_root_path, error_code);
  if(error_code){
    spdlog::error("Couldn't add directory {}. Error: {}", path.string(), error_code.message());
    return false;
  }

  if(relative_path.native()[0] == '.'){
    spdlog::error("Path {} is not a subpath of root.", path.string());
    return false;
  }

  // TODO: Add the file to the internal representation


  // Iterate through all the files
  for (auto directory_iterator = fs::directory_iterator(path);
       directory_iterator != fs::directory_iterator();
       ++directory_iterator)
  {
    
  }

  return false;
}
FileTree::FileTree(std::filesystem::path root_path)
    : m_root_path(std::move(root_path)) {
  // Check that the path is a directory
  if (!std::filesystem::is_directory(m_root_path)) {
    const auto message =
        fmt::format("The path {} is not a directory.", m_root_path.string());
    spdlog::error(message);
    throw std::invalid_argument(message);
  }

  // Create the file tree recursively
  add_directory(m_root_path, /*add_files=*/true, /*recursive=*/true);
}
auto FileTree::add_file(const std::filesystem::path& path) -> bool {
  return false;
}
auto FileTree::contains_path(const std::filesystem::path& path)
    -> std::optional<PathType> {
  return std::optional<PathType>();
}
}  // namespace album_architect::files