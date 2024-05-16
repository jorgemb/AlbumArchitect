//
// Created by Jorge on 08/05/2024.
//

#ifndef ALBUMARCHITECT_TREE_H
#define ALBUMARCHITECT_TREE_H

#include <cstdint>
#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/serialization/access.hpp>

namespace album_architect::files {

/// Represents the type of path
enum class PathType : std::uint8_t {
  file,
  directory
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
  static auto build(const boost::filesystem::path& path)
      -> std::optional<FileTree>;

  /// Builds a filetree from the provided stream
  /// \param input
  /// \return
  static auto from_stream(std::istream& input) -> std::optional<FileTree>;


  /// Writes the FileTree to the provided stream
  /// \param output
  /// \return
  void to_stream(std::ostream& output);


  /// Returns an optional PathType if the given path is part of the tree. In
  /// case it exists it returns the path type.
  /// \param path
  /// \return
  auto contains_path(const std::filesystem::path& path)
      -> std::optional<PathType>;

  /// Outputs a graphviz representation to the given stream
  /// \param ostream
  void to_graphviz(std::ostream& ostream) const;

  /// Allows for serialization via the << and >> operators using boost
  /// serialization framework.
  /// \tparam Archive
  /// \param archive
  /// \param version
  template<class Archive>
  void serialize(Archive& archive, const unsigned int version) {
    auto path = m_root_path.string();
    archive & path;
    archive & m_graph;
  }

  friend class boost::serialization::access;

  /// Comparison operators
  bool operator==(const FileTree& rhs) const;
  bool operator!=(const FileTree& rhs) const;

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
