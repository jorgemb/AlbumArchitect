//
// Created by jorge on 03/11/2023.
//

#ifndef ALBUMARCHITECT_ALBUM_H
#define ALBUMARCHITECT_ALBUM_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <cereal/cereal.hpp>

namespace album_architect {

// Forward declaration
class Photo;

/// \brief Contains all the metadata of the album
struct AlbumMetadata {
  std::map<std::string, std::shared_ptr<Photo>> photos;

  /// \brief Serialization for metadata
  /// \tparam Archive
  /// \param archive
  template<class Archive>
  void serialize(Archive& archive) {
    archive(CEREAL_NVP(photos));
  }
};

/// Represents a folder with Photos, Videos and other Albums
class Album {
  std::filesystem::path m_absolute_path;
  bool m_save_metadata;

  /// \brief Private default constructor
  explicit Album(std::filesystem::path absolute_path,
                 bool save_metadata = true);

  /// Cache for photos and albums
  mutable std::map<std::string, std::shared_ptr<Album>> m_albums;
  AlbumMetadata m_metadata;
  std::vector<std::string> m_files;

  /// \brief Loads the metadata file
  bool load_metadata();

  /// \brief Saves the metadata file
  void save_metadata();

public:
  /// Name of metadata file
  static inline const char* default_metadata_filename = ".albumarchitect";

  /// \brief Returns the absolute path associated to the Album
  /// \return
  [[nodiscard]] auto get_absolute_path() const -> const std::filesystem::path& {
    return m_absolute_path;
  }

  /// \brief Returns the name of the album (calculated from the path)
  /// \return
  [[nodiscard]] auto name() const -> std::string {
    return m_absolute_path.filename().string();
  }

  /// \brief Updates the internal cache of Photos and Albums
  auto update_album() -> void;

  /// \brief Returns all the files that are under the album, regardless of type.
  /// Does not include other folders / Albums
  /// \return List of paths
  [[nodiscard]] auto get_files() const -> std::vector<std::filesystem::path>;

  /// \brief Returns all detected photos under the Album
  /// \return
  [[nodiscard]] auto get_photos() const -> std::vector<std::shared_ptr<Photo>>;

  /// \brief Returns all detected Albums under the Album
  /// \return
  [[nodiscard]] auto get_albums() const -> std::vector<std::shared_ptr<Album>>;

  /// \brief Loads an Album at the specified path
  /// \return
  auto static load_album(const std::filesystem::path& album_path,
                         bool save_metadata = true) -> std::unique_ptr<Album>;
};

}  // namespace album_architect

#endif  // ALBUMARCHITECT_ALBUM_H
