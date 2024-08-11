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

  std::uint32_t width, height, channels;

  // NOLINTBEGIN(*-easily-swappable-parameters)
  ImageImpl(std::filesystem::path path,
            OIIO::ImageBuf image,
            const std::uint32_t width,
            const std::uint32_t height,
            const std::uint32_t channels)
      : path(std::move(path))
      , image(std::move(image))
      , width(width)
      , height(height)
      , channels(channels) {}
  // NOLINTEND(*-easily-swappable-parameters)
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

  // Get image information
  auto spec = loaded_image.spec();

  auto implementation = std::make_shared<ImageImpl>(
      path, loaded_image, spec.width, spec.height, spec.nchannels);
  return std::make_optional<Image>(implementation);
}
Image::Image(std::shared_ptr<ImageImpl> impl)
    : m_impl(std::move(impl)) {}
auto Image::get_width() const -> std::uint32_t {
  return m_impl->width;
}
auto Image::get_height() const -> std::uint32_t {
  return m_impl->height;
}
auto Image::get_channels() const -> std::uint32_t {
  return m_impl->channels;
}
auto Image::get_path() const -> std::filesystem::path {
  return m_impl->path;
}
}  // namespace albumarchitect::album