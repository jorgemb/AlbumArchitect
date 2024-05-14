//
// Created by Jorge on 08/05/2024.
//

#ifndef ALBUMARCHITECT_TREE_H
#define ALBUMARCHITECT_TREE_H

#include <cstdint>
#include <filesystem>
#include <optional>

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
  static auto build(const std::filesystem::path& path)
      -> std::optional<FileTree>;

  /// Returns an optional PathType if the given path is part of the tree. In
  /// case it exists it returns the path type.
  /// \param path
  /// \return
  auto contains_path(const std::filesystem::path& path)
      -> std::optional<PathType>;

  auto add_file(const std::filesystem::path& path) -> bool;

  /// Adds a directory that is a part of the filesystem
  /// \param path Path to use
  /// \param add_files Add files to the tree
  /// \param recursive Recursively add the sub-directories
  /// \return
  auto add_directory(const std::filesystem::path& path,
                     bool add_files,
                     bool recursive) -> bool;

  auto remove_file(const std::filesystem::path& path) -> bool;
  auto remove_directory(const std::filesystem::path& path) -> bool;

  auto move_file(const std::filesystem::path& old_path,
                 const std::filesystem::path& new_path) -> bool;
  auto move_directory(const std::filesystem::path& old_path,
                      const std::filesystem::path& new_path) -> bool;

  /// Outputs a graphviz representation to the given stream
  /// \param ostream
  void to_graphviz(std::ostream& ostream) const;

private:
  /// Recursively populates the tree with all the files and folders under the
  /// root path. Root path should be a directory. If a file is provided an
  /// exception is thrown.
  /// \param root_path
  explicit FileTree(std::filesystem::path root_path);

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
