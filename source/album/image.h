//
// Created by jorge on 09/08/24.
//

#ifndef IMAGE_H
#define IMAGE_H
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <opencv2/core/mat.hpp>

namespace album_architect::album {

/// Represents the different hashing algorithms currently supported
enum class HashAlgorithm : std::uint8_t {
  md5,
  sha256
};

/// Represents the different hashing algorithms for image
enum class ImageHashAlgorithm : std::uint8_t {
  average_hash,
  p_hash,
};

// Forward declaration
class ImageImpl;

/// General class to represent a single image in the drive
class Image {
public:
  /// Tries to load the image at the given path
  /// @param path
  /// @return
  static auto load(const std::filesystem::path& path) -> std::optional<Image>;

  /// Tries to determine if a given path contains an image that looks like it
  /// could be loaded.
  /// @param path
  /// @return
  static auto check_path_is_image(const std::filesystem::path& path) -> bool;

  /// Default constructor
  explicit Image(std::shared_ptr<ImageImpl> impl);

  /// Default constructor
  ~Image() = default;

  Image(const Image& other) = default;
  Image(Image&& other) noexcept = default;
  auto operator=(const Image& other) -> Image& = default;
  auto operator=(Image&& other) noexcept -> Image& = default;

  /// Width of the image in pixels
  /// @return
  auto get_width() const -> std::uint32_t;

  /// Height of the image in pixels
  /// @return
  auto get_height() const -> std::uint32_t;

  /// Number of channels in the image
  /// @return
  auto get_channels() const -> std::uint32_t;

  /// Path of the image
  /// @return
  auto get_path() const -> std::filesystem::path;

  /// Returns the map of metadata
  /// @return
  auto get_metadata() const -> const std::map<std::string, std::string>&;

  /// Returns the encoded hash representation for the image according to the
  /// algorithm
  /// @param algorithm
  /// @return
  auto get_hash(HashAlgorithm algorithm) const -> std::optional<std::string>;

  /// Returns the given algorithms as a CV2 mat
  /// @param algorithm
  /// @return
  auto get_image_hash(ImageHashAlgorithm algorithm) const -> cv::Mat;

  /// Returns the loaded image as a
  /// @return
  auto get_image(cv::Mat& output) const -> bool;

private:
  std::shared_ptr<ImageImpl> m_impl;
};

}  // namespace album_architect::album

#endif  // IMAGE_H
