//
// Created by jorge on 11/10/2023.
//

#ifndef ALBUMARCHITECT_PHOTO_H
#define ALBUMARCHITECT_PHOTO_H

#include <filesystem>
#include <optional>
#include <openimageio/imagebuf.h>

namespace album_architect
{

class Photo{
public:
  /// Tries to create a new photo from the given path. Returns
  /// \param path
  /// \return
  static auto load(const std::filesystem::path& path) -> std::unique_ptr<Photo>;

  auto get_path() const -> const std::filesystem::path&;
  auto get_width() const -> int64_t;
  auto get_height() const -> int64_t;

  /// Destructor
  virtual ~Photo() = default;

  // Constructors
  Photo(const Photo& other) = default;
  Photo(Photo&& other) noexcept = default;
  auto operator=(const Photo& other) -> Photo& = default;
  auto operator=(Photo&& other) noexcept -> Photo& = default;

private:
  /// Path of the photo
  std::filesystem::path m_path;

  /// Internal photo
  OIIO::ImageBuf m_image;

  Photo(std::filesystem::path path,
        OIIO::ImageBuf  image);

};



}  // namespace album_architect

#endif  // ALBUMARCHITECT_PHOTO_H
