//
// Created by Jorge on 14/05/2024.
//

#ifndef ALBUMARCHITECT_HELPER_H
#define ALBUMARCHITECT_HELPER_H

#include <filesystem>

namespace album_architect::files {

/// This class helps to set the current directory for a temporary value. Once
/// the class is deleted the previous directory is restored
class TempCurrentDir{
public:
  /// Sets the current directory to the given value, restoring on delete.
  /// \param new_current_directory
  TempCurrentDir(const std::filesystem::path& new_current_directory);

  /// Restores the current dir to a new value
  ~TempCurrentDir();
private:
  std::filesystem::path m_original_path;
};

} // namespace album_architect::files

#endif  // ALBUMARCHITECT_HELPER_H
