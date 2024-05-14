//
// Created by Jorge on 14/05/2024.
//

#include <filesystem>

#include "helper.h"

#include <spdlog/spdlog.h>

namespace album_architect::files {

namespace fs = std::filesystem;
TempCurrentDir::TempCurrentDir(
    const std::filesystem::path& new_current_directory)
    : m_original_path(fs::current_path()) {
  auto has_error = std::error_code {};
  fs::current_path(new_current_directory, has_error);

  // Check error code
  if (has_error) {
    spdlog::error("Couldn't set the current directory to {}. Error: {}",
                  new_current_directory.string(),
                  has_error.message());
  }
}
TempCurrentDir::~TempCurrentDir() {
  auto has_error = std::error_code {};
  fs::current_path(m_original_path, has_error);

  if (has_error) {
    spdlog::error("Couldn't restore to the previous directory. Error: {}",
                  has_error.message());
  }
}
}  // namespace album_architect::files