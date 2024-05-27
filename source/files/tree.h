//
// Created by Jorge on 08/05/2024.
//

#ifndef ALBUMARCHITECT_TREE_H
#define ALBUMARCHITECT_TREE_H

#include <cstdint>
#include <filesystem>
#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include <boost/serialization/split_member.hpp>

namespace album_architect::files {

/// Represents the type of path
enum class PathType : std::uint8_t {
  invalid = 0,
  file,
  directory
};

/// Forward declarations
class FileTree;

/// Represents an element from within the FileTree
class Element {
public:
  using ElementList = std::vector<Element>;

  /// Creates a new Element
  /// \param type
  /// \param path
  /// \param parent
  Element(PathType type, const std::filesystem::path& path, FileTree* parent);

  // Getters
  auto get_type() const -> PathType;
  auto get_path() const -> const std::filesystem::path&;

  // Comparison
  auto operator==(const Element& rhs) const -> bool;
  auto operator!=(const Element& rhs) const -> bool;

  // Get different elements relative from this

  /// Returns the elements that are at the same level as this element.
  /// \return
  auto get_siblings() const -> ElementList;

  /// If this Element is a directory, then returns all direct elements that are
  /// children of this
  /// \return
  auto get_children() const -> ElementList;

  /// Returns the parent of this element. The root object doesn't have a parent.
  /// \return
  auto get_parent() const -> std::optional<Element>;

private:
  PathType m_type;
  std::filesystem::path m_path;
  FileTree* m_parent;
};

/// Represents a tree from the filesystem, mirroring the values inside
class FileTree {
public:
  /// Destructor has to be visible to support forward declarations.
  ~FileTree();

  // Disable copy
  FileTree(const FileTree& other) = delete;
  auto operator=(const FileTree& other) = delete;

  // Default move
  FileTree(FileTree&& other) = default;
  auto operator=(FileTree&& other) -> FileTree& = default;

  /// Creates a file tree from the specified root directory.
  /// \param path Path to a directory to search recursively.
  /// \return A file tree, or None if a directory is not specified
  static auto build(const std::filesystem::path& path)
      -> std::optional<FileTree>;

  /// Builds a filetree from the provided stream
  /// \param input
  /// \return
  static auto from_stream(std::istream& input) -> std::optional<FileTree>;

  /// Writes the FileTree to the provided stream
  /// \param output
  /// \return
  void to_stream(std::ostream& output) const;

  /// Returns an optional PathType if the given path is part of the tree. In
  /// case it exists it returns the path type.
  /// \param path
  /// \return
  auto get_element(const std::filesystem::path& path) -> std::optional<Element>;

  /// Returns the elements under the path
  /// \param path
  /// \param output
  /// \return True if the path is within the tree, false otherwise
  auto get_elements_under_path(const std::filesystem::path& path,
                               std::vector<Element>& output) -> bool;

  /// Outputs a graphviz representation to the given stream
  /// \param ostream
  void to_graphviz(std::ostream& ostream) const;

  /// Allows for serialization via the << and >> operators using boost
  /// serialization framework.
  /// \tparam Archive
  /// \param archive
  /// \param version
  template<class Archive>
  void load(Archive& archive, const unsigned int /* version */) {
    auto str_path = std::string {};
    archive & str_path;
    m_root_path = str_path;

    archive & m_graph;
  }

  /// Allows for serialization via the << and >> operators using boost
  /// serialization framework.
  /// \tparam Archive
  /// \param archive
  /// \param version
  template<class Archive>
  void save(Archive& archive, const unsigned int /* version */) const {
    auto str_path = m_root_path.string();

    archive & str_path;
    archive & m_graph;
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

  /// Comparison operators
  auto operator==(const FileTree& rhs) const -> bool;
  auto operator!=(const FileTree& rhs) const -> bool;

private:
  /// Adds a directory that is a part of the filesystem
  /// \param path Path to use
  /// \param add_files Add files to the tree
  /// \param recursive Recursively add the sub-directories
  /// \return
  auto add_directory(const std::filesystem::path& path,
                     bool add_files,
                     bool recursive) -> bool;

  /// Recursively populates the tree with all the files and folders under the
  /// root path. Root path should be a directory. If a file is provided an
  /// exception is thrown.
  /// \param root_path
  explicit FileTree(std::filesystem::path root_path);

  /// Default constructor to allow building from a stream
  FileTree() = default;

  /// Returns true if the given path is a sub-path of root
  /// \param path
  /// \return
  auto is_subpath(const std::filesystem::path& path) -> bool;

  /// Converts a path into a path list
  /// \param path
  /// \return
  auto to_path_list(const std::filesystem::path& path)
      -> std::vector<std::string>;

  std::filesystem::path m_root_path;

  std::unique_ptr<class FileGraph> m_graph;
};

}  // namespace album_architect::files

#endif  // ALBUMARCHITECT_TREE_H
