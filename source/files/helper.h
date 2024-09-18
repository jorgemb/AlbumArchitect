//
// Created by Jorge on 14/05/2024.
//

#ifndef ALBUMARCHITECT_FILES_HELPER_H
#define ALBUMARCHITECT_FILES_HELPER_H

#include <filesystem>
#include <string>

#include <boost/serialization/optional.hpp>

namespace album_architect::files {

/// This class helps to set the current directory for a temporary value. Once
/// the class is deleted the previous directory is restored
class TempCurrentDir {
public:
  /// Sets the current directory to the given value, restoring on delete.
  /// \param new_current_directory
  explicit TempCurrentDir(const std::filesystem::path& new_current_directory);

  /// Restores the current dir to a new value
  ~TempCurrentDir();

  // Deleted operators
  TempCurrentDir(const TempCurrentDir& other) = delete;
  auto operator=(const TempCurrentDir& other) = delete;

  // Move operators
  TempCurrentDir(TempCurrentDir&& other) = default;
  auto operator=(TempCurrentDir&& other) -> TempCurrentDir& = default;
private:
  std::filesystem::path m_original_path;
};

/// Creates a temporary file that will be deleted once the class is deleted
class TemporaryFile {
public:
  /// Creates a temporary file with the optional name
  /// \param name
  TemporaryFile();

  /// Deletes the newly created file
  ~TemporaryFile();

  /// Deleted operators
  TemporaryFile(const TemporaryFile& other) = delete;
  auto operator=(const TemporaryFile& other) = delete;

  /// Default operators
  TemporaryFile(TemporaryFile&& other) = default;
  auto operator=(TemporaryFile&& other) -> TemporaryFile& = default;

  /// Returns the associated path
  /// \return
  auto get_path() const -> std::filesystem::path;

private:
  std::filesystem::path m_path;
};

}  // namespace album_architect::files

#endif  // ALBUMARCHITECT_FILES_HELPER_H
