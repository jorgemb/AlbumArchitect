//
// Created by jorge on 11/10/2023.
//

#include <utility>

#include "photo.h"

#include <OpenImageIO/imagebuf.h>
#include <glog/logging.h>

namespace album_architect {

auto Photo::load(const std::filesystem::path& path) -> std::unique_ptr<Photo> {
  // Check if path exists
  if (!exists(path)) {
    LOG(ERROR) << "Path doesn't exist: " << path;
    return {};
  }

  // Load the photo
  auto image = OIIO::ImageBuf(path.string());
  if (!image.initialized()) {
    // Error while reading image
    LOG(ERROR) << "Couldn't get image information for: " << path
               << ". Error: " << image.geterror();
    return {};
  }

  return std::unique_ptr<Photo>(new Photo(path, std::move(image)));
}

auto Photo::get_path() const -> const std::filesystem::path& {
  return m_path;
}
auto Photo::get_width() const -> int64_t {
  return m_image.spec().width;
}
auto Photo::get_height() const -> int64_t {
  return m_image.spec().height;
}
Photo::Photo(std::filesystem::path path,
             OIIO::ImageBuf image)
    : m_path(std::move(path))
    , m_image(std::move(image)) {}

}  // namespace album_architect