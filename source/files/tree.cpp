//
// Created by Jorge on 08/05/2024.
//

#include <exception>
#include <filesystem>
#include <utility>

#include "tree.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <iostream>

#include "graph.h"

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
  if (path != m_root_path && !is_subpath(path)) {
    spdlog::error("Path {} is not a subpath of root.", path.string());
    return false;
  }

  // Check that it is a directory
  if(!std::filesystem::is_directory(path)){
    spdlog::error("Path {} is not a directory.", path.string());
    return false;
  }

  // Add the file to the internal representation
  auto relative_path = std::filesystem::relative(path, m_root_path);
  auto path_list = to_path_list(relative_path);
  if(path != m_root_path) {
    m_graph->add_node(path_list, NodeType::directory);
  }


  // Iterate through all the files
  for (auto directory_iterator = fs::directory_iterator(m_root_path / relative_path);
       directory_iterator != fs::directory_iterator();
       ++directory_iterator)
  {
    // Check if it is directory or file
    if(directory_iterator->is_directory() && recursive){
      add_directory(directory_iterator->path(), add_files, recursive);
    } else if (directory_iterator->is_regular_file() && add_files){
      path_list.push_back(directory_iterator->path().filename().string());
      m_graph->add_node(path_list, NodeType::file);
      path_list.pop_back();
    }

  }

  return true;
}
FileTree::FileTree(std::filesystem::path root_path)
    : m_root_path(std::move(root_path))
    , m_graph(std::make_unique<FileGraph>()) {
  // Check that the path is a directory
  if (!std::filesystem::is_directory(m_root_path)) {
    const auto message =
        fmt::format("The path {} is not a directory.", m_root_path.string());
    spdlog::error(message);
    throw std::invalid_argument(message);
  }

  // Create the file tree recursively
  add_directory(m_root_path, /*add_files=*/true, /*recursive=*/true);

  // DEBUG: Show full tree
  m_graph->to_graphviz(std::cout);
}
auto FileTree::add_file(const std::filesystem::path& path) -> bool {
  return false;
}
auto FileTree::contains_path(const std::filesystem::path& path)
    -> std::optional<PathType> {
  // Check path
  if(!is_subpath(path)){
    return {};
  }

  auto relative_path = std::filesystem::relative(path, m_root_path);
  auto path_list = to_path_list(relative_path);
  auto node = m_graph->get_node_type(path_list);

  if(!node){
    return {};
  }

  switch (node.value()) {
    case NodeType::directory:
      return PathType::directory;
    case NodeType::file:
      return PathType::file;
  }

  throw std::runtime_error("Unreachable");
}
auto FileTree::is_subpath(const std::filesystem::path& path) -> bool {
  auto error_code = std::error_code {};
  auto relative_path = fs::relative(path, m_root_path, error_code);
  if (error_code) {
    spdlog::error("Couldn't add directory {}. Error: {}",
                  path.string(),
                  error_code.message());
    return false;
  }

  return !relative_path.empty() && relative_path.native()[0] != '.';
}
auto FileTree::to_path_list(const std::filesystem::path& path)
    -> std::vector<std::string> {
  // Check if path is empty
  if (path.empty()) {
    return {};
  }

  auto path_list = std::vector<std::string> {path.filename().string()};

  auto current_path = path.parent_path();
  while (!current_path.empty()) {
    path_list.push_back(current_path.filename().string());
    current_path = current_path.parent_path();
  }

  std::reverse(path_list.begin(), path_list.end());
  return path_list;
}
}  // namespace album_architect::files