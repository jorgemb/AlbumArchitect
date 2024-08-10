//
// Created by jorge on 09/08/24.
//

#include <utility>

#include "image.h"

#include <OpenImageIO/imagebuf.h>
#include <spdlog/spdlog.h>

namespace albumarchitect::album {

/// Implementation for image
class ImageImpl {
public:
  std::filesystem::path path;
  OIIO::ImageBuf image;

  ImageImpl(std::filesystem::path image_path, OIIO::ImageBuf image_buf)
      : path(std::move(image_path))
      , image(std::move(image_buf)) {}
};

auto Image::load(const std::filesystem::path& path) -> std::optional<Image> {
  // Try loading the image
  auto loaded_image = OIIO::ImageBuf(path.string());
  if (!loaded_image.initialized()) {
    spdlog::error("Couldn´t load image at {}. Reason: {}",
                  path.string(),
                  loaded_image.geterror());
    return {};
  }

  auto implementation = std::make_shared<ImageImpl>(path, loaded_image);
  return std::make_optional<Image>(implementation);
  return {};
}
Image::Image(std::shared_ptr<ImageImpl> impl)
    : m_impl(std::move(impl)) {}
}  // namespace albumarchitect::album