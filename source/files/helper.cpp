//
// Created by Jorge on 14/05/2024.
//

#include <filesystem>

#include "helper.h"

#include <boost/filesystem/operations.hpp>
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
  if(m_original_path.empty()){
    // Don't do anything, the object was probably moved
  }

  auto has_error = std::error_code {};
  fs::current_path(m_original_path, has_error);

  if (has_error) {
    spdlog::error("Couldn't restore to the previous directory. Error: {}",
                  has_error.message());
  }
}
TemporaryFile::TemporaryFile() {
  auto path = boost::filesystem::unique_path();
  m_path = std::filesystem::temp_directory_path() / path.string();
}
TemporaryFile::~TemporaryFile() {
  try {
    if (std::filesystem::exists(m_path)) {
      auto status = std::filesystem::remove(m_path);
      if (!status) {
        spdlog::error("Couldn't delete temporary file: {}", m_path.string());
      }
    }
  } catch (const std::filesystem::filesystem_error& ex) {
    spdlog::error(
        "Exception while handling removal of temporary file: {}. Error: {}",
        m_path.string(),
        ex.what());
  }
}
auto TemporaryFile::get_path() const -> std::filesystem::path {
  return m_path;
}
}  // namespace album_architect::files