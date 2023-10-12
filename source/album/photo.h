//
// Created by jorge on 11/10/2023.
//

#ifndef ALBUMARCHITECT_PHOTO_H
#define ALBUMARCHITECT_PHOTO_H

#include <filesystem>
#include <optional>

namespace album_architect
{

class Photo{
public:
  /// Tries to create a new photo from the given path. Returns
  /// \param path
  /// \return
  static auto load(const std::filesystem::path& path) -> std::optional<Photo>;

  /// \return Path of the photo
  auto get_path() const -> const std::filesystem::path&;
  auto get_width() const -> int64_t;
  auto get_height() const -> int64_t;
private:
  /// Photo can only be created via the load method
  Photo(std::filesystem::path path, int64_t width, int64_t height);

  /// Path of the photo
  std::filesystem::path m_path;

  /// Dimensions of the photo
  int64_t m_width, m_height;
};



}  // namespace album_architect

#endif  // ALBUMARCHITECT_PHOTO_H
