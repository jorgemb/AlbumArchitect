//
// Created by jorge on 09/08/24.
//

#include "image.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

namespace albumarchitect::album {

auto Image::load(const std::filesystem::path& path) -> std::optional<Image> {
  // Try loading the image
  auto loaded_image = cv::imread(path.string(), cv::IMREAD_COLOR);
  if (loaded_image.empty()) {
    return {};
  }

  return Image {path};
}
Image::Image(std::filesystem::path path)
    : m_path(std::move(path)) {}
}  // namespace albumarchitect::album