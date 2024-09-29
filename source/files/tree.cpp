//
// Created by Jorge on 08/05/2024.
//

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "files/tree.h"

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "files/common.h"
#include "files/graph.h"
#include "files/helper.h"

namespace fs = std::filesystem;

namespace album_architect::files {

auto FileTree::build(const std::filesystem::path& path)
    -> std::optional<FileTree> {
  // Check if path is directory or file
  if (!fs::is_directory(path)) {
    spdlog::error(
        "Cannot create a FileTree on path {}. Path should be a directory.",
        path.string());
    return {};
  }

  auto has_error = std::error_code {};
  auto absolute_path = fs::absolute(path, has_error);
  if (has_error) {
    spdlog::error("Couldn't convert path {} to absolute. Error: {}",
                  path.string(),
                  has_error.message());
    return {};
  }

  return FileTree {absolute_path};
}

FileTree::FileTree(std::filesystem::path root_path)
    : m_root_path(std::move(root_path))
    , m_graph(std::make_unique<FileGraph>(true)) {
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
auto FileTree::get_element(const std::filesystem::path& path)
    -> std::optional<Element> {
  // On empty path return the root
  if (path.empty()) {
    return std::make_optional<Element>(PathType::directory, m_root_path, this);
  }

  // Check path
  if (!is_subpath(path)) {
    return {};
  }

  auto relative_path = std::filesystem::relative(path, m_root_path);
  auto path_list = to_path_list(relative_path);
  auto node = m_graph->get_node(path_list);

  if (!node) {
    return {};
  }

  // Convert from NodeType to PathType
  auto path_type = from_node_type(m_graph->get_node_type(*node));

  return std::make_optional<Element>(path_type, fs::absolute(path), this);
}

// NOLINTNEXTLINE(misc-no-recursion)
auto FileTree::add_directory(const std::filesystem::path& path,
                             bool add_files,
                             bool recursive) -> bool {
  // Check that directory is subdirectory of root
  if (path != m_root_path && !is_subpath(path)) {
    spdlog::error("Path {} is not a subpath of root.", path.string());
    return false;
  }

  // Check that it is a directory
  if (!std::filesystem::is_directory(path)) {
    spdlog::error("Path {} is not a directory.", path.string());
    return false;
  }

  // Set root dir as current dir
  auto temporary_cwd = TempCurrentDir(m_root_path);

  // Add the file to the internal representation
  auto relative_path = std::filesystem::relative(path, m_root_path);
  auto path_list = to_path_list(relative_path);
  if (path != m_root_path) {
    m_graph->add_node(path_list, NodeType::directory);
  } else {
    // Clear the path list to not include "."
    path_list.clear();
  }

  // Iterate through all the files
  for (auto directory_iterator = fs::directory_iterator(relative_path);
       directory_iterator != fs::directory_iterator();
       ++directory_iterator)
  {
    // Check if it is directory or file
    if (directory_iterator->is_directory() && recursive) {
      add_directory(directory_iterator->path(), add_files, recursive);
    } else if (directory_iterator->is_regular_file() && add_files) {
      path_list.push_back(directory_iterator->path().filename().string());
      m_graph->add_node(path_list, NodeType::file);
      path_list.pop_back();
    }
  }

  return true;
}
auto FileTree::is_subpath(const std::filesystem::path& path) const -> bool {
  if (path == m_root_path) {
    return true;
  }

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
void FileTree::to_graphviz(std::ostream& ostream) const {
  m_graph->to_graphviz(ostream);
}
auto FileTree::operator==(const FileTree& rhs) const -> bool {
  return m_root_path == rhs.m_root_path && *m_graph == *rhs.m_graph;
}
auto FileTree::operator!=(const FileTree& rhs) const -> bool {
  return !(rhs == *this);
}
void FileTree::to_stream(std::ostream& output) const {
  auto archiver = boost::archive::binary_oarchive {output};
  archiver << *this;
}
auto FileTree::from_stream(std::istream& input) -> std::optional<FileTree> {
  auto archiver = boost::archive::binary_iarchive {input};
  try {
    auto new_tree = files::FileTree {};
    archiver >> new_tree;

    return std::make_optional(std::move(new_tree));
  } catch (const boost::archive::archive_exception& e) {
    spdlog::error("Couldn't deserialize FileTree. Error: {}", e.what());
    return {};
  }
}
auto FileTree::get_elements_under_path(const std::filesystem::path& path,
                                       std::vector<Element>& output) -> bool {
  // Check if path belongs to the tree
  if (!is_subpath(path)) {
    return false;
  }

  // Get the children under the path
  auto relative_path = fs::relative(path, m_root_path);
  auto path_list = to_path_list(relative_path);
  auto node = m_graph->get_node(path_list);
  if (!node) {
    return false;
  }

  auto children = m_graph->get_node_children(*node);

  // Create the Elements from the children
  std::transform(children.begin(),
                 children.end(),
                 std::back_inserter(output),
                 [this](auto child)
                 {
                   auto type = m_graph->get_node_type(child);
                   auto current_path_list = m_graph->get_node_path(child);

                   auto current_path = m_root_path;
                   for (auto&& element : current_path_list) {
                     current_path.append(std::move(element));
                   }

                   return Element {from_node_type(type), current_path, this};
                 });

  return true;
}
auto FileTree::get_root_element() -> Element {
  return {PathType::directory, m_root_path, this};
}
auto FileTree::set_metadata(const std::filesystem::path& path,
                            const std::string& key,
                            const PathAttribute& attribute)
    -> std::optional<PathAttribute> {
  // Check if path belongs to the tree
  if (!is_subpath(path)) {
    return {};
  }
  auto relative_path = fs::relative(path, m_root_path);
  auto path_list = to_path_list(relative_path);
  auto node = m_graph->get_node(path_list);
  if (!node) {
    return {};
  }

  return m_graph->set_node_metadata(node.value(), key, attribute);
}
auto FileTree::get_metadata(const std::filesystem::path& path,
                            const std::string& key) const
    -> std::optional<PathAttribute> {
  // Check if path belongs to the tree
  if (!is_subpath(path)) {
    return {};
  }
  auto relative_path = fs::relative(path, m_root_path);
  auto path_list = to_path_list(relative_path);
  auto node = m_graph->get_node(path_list);
  if (!node) {
    return {};
  }

  // Retrieve the node metadata
  return m_graph->get_node_metadata(node.value(), key);
}
auto FileTree::remove_metadata(const std::filesystem::path& path,
                               const std::string& key)
    -> std::optional<PathAttribute> {
  // Check if path belongs to the tree
  if (!is_subpath(path)) {
    return {};
  }
  auto relative_path = fs::relative(path, m_root_path);
  auto path_list = to_path_list(relative_path);
  auto node = m_graph->get_node(path_list);
  if (!node) {
    return {};
  }

  return m_graph->remove_node_metadata(node.value(), key);
}
Element::Element(PathType type, std::filesystem::path path, FileTree* parent)
    : m_type(type)
    , m_path(std::move(path))
    , m_parent(parent) {}
auto Element::get_type() const -> PathType {
  return m_type;
}
auto Element::get_path() const -> const std::filesystem::path& {
  return m_path;
}
auto Element::operator==(const Element& rhs) const -> bool {
  return m_type == rhs.m_type && m_path == rhs.m_path
      && m_parent == rhs.m_parent;
}
auto Element::operator!=(const Element& rhs) const -> bool {
  return !(rhs == *this);
}
auto Element::get_children() const -> Element::ElementList {
  auto elements = ElementList {};
  m_parent->get_elements_under_path(get_path(), elements);
  return elements;
}
auto Element::get_parent() const -> std::optional<Element> {
  auto own_path = get_path();
  if (!own_path.has_parent_path()) {
    return {};
  }

  auto parent_path = own_path.parent_path();
  return m_parent->get_element(parent_path);
}
auto Element::get_siblings() const -> Element::ElementList {
  auto siblings = ElementList {};

  // Get the children of the parent
  auto parent = get_parent();
  if (!parent) {
    return {};
  }

  auto children = parent->get_children();
  siblings.reserve(children.size());

  // Remove oneself from the sibling list
  auto own_path = get_path();
  std::copy_if(children.begin(),
               children.end(),
               std::back_inserter(siblings),
               [&own_path](auto element)
               { return element.get_path() != own_path; });

  return siblings;
}
auto Element::set_metadata(const std::string& key,
                           const PathAttribute& attribute)
    -> std::optional<PathAttribute> {
  return m_parent->set_metadata(m_path, key, attribute);
}
auto Element::get_metadata(const std::string& key) const
    -> std::optional<PathAttribute> {
  return m_parent->get_metadata(m_path, key);
}
auto Element::remove_metadata(const std::string& key)
    -> std::optional<PathAttribute> {
  return m_parent->remove_metadata(m_path, key);
}
auto from_node_type(const NodeType& path_type) -> PathType {
  switch (path_type) {
    case NodeType::directory:
      return PathType::directory;
    case NodeType::file:
      return PathType::file;
    default:
      return PathType::invalid;
  }
}
}  // namespace album_architect::files