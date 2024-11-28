//
// Created by jorge on 09/08/24.
//

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "image.h"

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imageio.h>
#include <spdlog/spdlog.h>

#include "album/hash.h"

namespace album_architect::album {

/// Implementation for image
class ImageImpl {
public:
  std::filesystem::path path;
  OIIO::ImageBuf image;

  std::uint32_t width, height, channels;

  std::map<std::string, std::string> metadata;

  // NOLINTBEGIN(*-easily-swappable-parameters)
  ImageImpl(std::filesystem::path path,
            OIIO::ImageBuf image,
            const std::uint32_t width,
            const std::uint32_t height,
            const std::uint32_t channels,
            std::map<std::string, std::string>&& metadata)
      : path(std::move(path))
      , image(std::move(image))
      , width(width)
      , height(height)
      , channels(channels)
      , metadata(std::move(metadata)) {}
  // NOLINTEND(*-easily-swappable-parameters)
};

auto Image::load(const std::filesystem::path& path) -> std::optional<Image> {
  // Try loading the image
  auto loaded_image = OIIO::ImageBuf(path.string());
  if (!loaded_image.initialized()) {
    spdlog::error("Couldn't load image at {}. Reason: {}",
                  path.string(),
                  loaded_image.geterror());
    return {};
  }

  // Get image information
  auto spec = loaded_image.spec();

  // Get metadata
  auto metadata = std::map<std::string, std::string> {};
  std::for_each(spec.extra_attribs.begin(),
                spec.extra_attribs.end(),
                [&metadata](const auto& attrib)
                { metadata.emplace(attrib.name(), attrib.get_string()); });

  auto implementation = std::make_shared<ImageImpl>(path,
                                                    loaded_image,
                                                    spec.width,
                                                    spec.height,
                                                    spec.nchannels,
                                                    std::move(metadata));
  return std::make_optional<Image>(implementation);
}
auto Image::check_path_is_image(const std::filesystem::path& path) -> bool {
  auto image_input = OIIO::ImageInput::create(path.string());
  return static_cast<bool>(image_input);
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
auto Image::get_metadata() const -> const std::map<std::string, std::string>& {
  return m_impl->metadata;
}
auto Image::get_hash(const HashAlgorithm algorithm) const
    -> std::optional<std::string> {
  switch (algorithm) {
    case HashAlgorithm::md5:
      return hash::Hash::calculate_md5(m_impl->path);
    case HashAlgorithm::sha256:
      return hash::Hash::calculate_sha256(m_impl->path);
    default:
      return {};
  }
}
auto Image::get_image_hash(ImageHashAlgorithm algorithm) const -> cv::Mat {
  auto mat = cv::Mat {};
  get_image(mat);

  switch (algorithm) {
    case ImageHashAlgorithm::average_hash:
      return hash::Hash::calculate_average_hash(mat);
    case ImageHashAlgorithm::p_hash:
      return hash::Hash::calculate_p_hash(mat);
    default:
      return {};
  }
}
auto Image::get_image(cv::Mat& output) const -> bool {
  if (OIIO::ImageBufAlgo::to_OpenCV(output, m_impl->image)) {
    return true;
  }

  // Log error
  spdlog::error("Error getting cv::Mat for image {}. Error: {}",
                m_impl->path.string(),
                OIIO::geterror());
  return false;
}
}  // namespace album_architect::album