//
// Created by jorge on 09/08/24.
//

#ifndef IMAGE_H
#define IMAGE_H
#include <filesystem>
#include <map>
#include <optional>
#include <string>

namespace albumarchitect::album {

// Forward declaration
class ImageImpl;

/// General class to represent a single image in the drive
class Image {
public:
  /// Tries to load the image at the given path
  /// @param path
  /// @return
  static auto load(const std::filesystem::path& path) -> std::optional<Image>;

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

private:
  std::shared_ptr<ImageImpl> m_impl;
};

}  // namespace albumarchitect::album

#endif  // IMAGE_H
