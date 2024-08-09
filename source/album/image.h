//
// Created by jorge on 09/08/24.
//

#ifndef IMAGE_H
#define IMAGE_H
#include <filesystem>
#include <optional>

#include <opencv2/core/mat.hpp>

namespace albumarchitect::album {

/// General class to represent a single image in the drive
class Image {
public:
  /// Tries to load the image at the given path
  /// @param path
  /// @return
  static auto load(const std::filesystem::path& path) -> std::optional<Image>;

private:
  /// Default constructor private, so ::load has to be used
  explicit Image(std::filesystem::path path);

  // Path to the image
  std::filesystem::path m_path;

  // Internal representation of image
  cv::Mat m_mat;
};

}  // namespace albumarchitect::album

#endif  // IMAGE_H
