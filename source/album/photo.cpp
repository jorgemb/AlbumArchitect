//
// Created by jorge on 11/10/2023.
//

#include <utility>

#include "photo.h"

#include <glog/logging.h>
#include <imageinfo.hpp>

namespace album_architect {
auto Photo::load(const std::filesystem::path& path) -> std::optional<Photo> {
  // Check if path exists
  if (!exists(path)) {
    LOG(ERROR) << "Path doesn't exist: " << path;
    return {};
  }

  // Get information about the photo
  auto info = getImageInfo<IIFilePathReader>(path.string());
  if (info.getErrorCode() != II_ERR_OK) {
    // Error while reading image
    LOG(ERROR) << "Couldn't get image information for: " << path
               << ". Error: " << info.getErrorMsg();
    return {};
  }

  // Create photo
  return std::make_optional(Photo(path, info.getWidth(), info.getHeight()));
}
auto Photo::get_path() const -> const std::filesystem::path& {
  return m_path;
}
auto Photo::get_width() const -> int64_t {
  return m_width;
}
auto Photo::get_height() const -> int64_t {
  return m_height;
}
Photo::Photo(std::filesystem::path  path, int64_t width, int64_t height)
    : m_path(std::move(path))
    , m_width(width)
    , m_height(height) {}
}  // namespace album_architect