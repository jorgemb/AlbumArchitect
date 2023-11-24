//
// Created by jorge on 03/11/2023.
//

#include <fstream>
#include <ranges>
#include <set>

#include "album.h"

#include <album/photo.h>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <glog/logging.h>

namespace album_architect {

namespace fs = std::filesystem;
namespace ranges = std::ranges;
namespace views = std::ranges::views;

Album::Album(std::filesystem::path absolute_path, bool save_metadata)
    : m_absolute_path(std::move(absolute_path))
    , m_save_metadata(save_metadata) {
  if (!load_metadata()) {
    update_album();
  }
}
bool Album::load_metadata() {
  const auto metadata_path = m_absolute_path / default_metadata_filename;
  auto metadata_file = std::ifstream(metadata_path, std::ios::binary);
  if (!metadata_file.is_open()) {
    return false;
  }

  // Try loading the metadata
  auto archive = cereal::JSONInputArchive(metadata_file);
  archive(m_metadata);

  // Remove all images that don't exist anymore
  for (auto photo_iter = m_metadata.photos.begin();
       photo_iter != m_metadata.photos.end();
       /* no update */)
  {
    if (photo_iter->second->is_ok()) {
      ++photo_iter;
    } else {
      // Remove the invalid photo
      photo_iter = m_metadata.photos.erase(photo_iter);
    }
  }

  return true;
}
void Album::save_metadata() {
  // Check if allowed to save metadata
  if (!m_save_metadata)
    return;

  const auto metadata_path = m_absolute_path / default_metadata_filename;
  auto metadata_file = std::ofstream(metadata_path, std::ios::binary);
  if (metadata_file.is_open()) {
    // Try saving the metadata
    auto archive = cereal::JSONOutputArchive(metadata_file);
    archive(m_metadata);
  } else {
    LOG(ERROR) << "Couldn't save metadata for album at " << m_absolute_path;
  }
}
auto Album::update_album() -> void {
  DLOG(INFO) << "Updating album at: " << m_absolute_path;

  m_files.clear();

  auto processed_albums = std::set<std::string> {};
  auto processed_photos = std::set<std::string> {};

  // Get all files under the directory
  for (const auto& entry : fs::directory_iterator(m_absolute_path)) {
    auto current_file = entry.path().filename().string();

    // Check if it is a photo or album
    if (entry.is_directory()) {
      // .. add Album
      processed_albums.insert(current_file);
      if (!m_albums.contains(current_file)) {
        // New Album found
        DLOG(INFO) << "--Album found at " << current_file;
        m_albums[current_file] = {};
      }
    } else {
      m_files.push_back(current_file);
      // .. add File, probable Photo
      if (!m_metadata.photos.contains(current_file)) {
        // Try to load as Photo
        auto photo = std::make_unique<Photo>(entry.path());
        if (photo->is_ok()) {
          DLOG(INFO) << "--Photo found at " << current_file;
          processed_photos.insert(current_file);
          m_metadata.photos[current_file] = std::move(photo);
        }
      } else {
        processed_photos.insert(current_file);
      }
    }
  }

  // Check if there are photos / albums that are not in the list
  for (auto current_album = m_albums.begin(); current_album != m_albums.end();
       /* no increment */)
  {
    if (!processed_albums.contains(current_album->first)) {
      // Delete album
      DLOG(INFO) << "--Removed album at " << current_album->first;
      current_album = m_albums.erase(current_album);
    } else {
      ++current_album;
    }
  }

  for (auto current_photo = m_metadata.photos.begin();
       current_photo != m_metadata.photos.end();
       /* no increment */)
  {
    if (!processed_photos.contains(current_photo->first)) {
      // Delete photo
      DLOG(INFO) << "--Removed photo at " << current_photo->first;
      current_photo = m_metadata.photos.erase(current_photo);
    } else {
      ++current_photo;
    }
  }

  save_metadata();
}
auto Album::get_files() const -> std::vector<std::filesystem::path> {
  auto result = std::vector<std::filesystem::path> {};
  ranges::transform(m_files,
                    std::back_inserter(result),
                    [this](const auto& name)
                    { return m_absolute_path / name; });

  return result;
}
auto Album::get_photos() const -> std::vector<std::shared_ptr<Photo>> {
  auto result = std::vector<std::shared_ptr<Photo>> {};
  result.reserve(m_files.size());

  ranges::transform(m_metadata.photos,
                    std::back_inserter(result),
                    [](const auto& photo_element)
                    { return photo_element.second; });

  return result;
}
auto Album::get_albums() const -> std::vector<std::shared_ptr<Album>> {
  // Load all directories from this Album
  auto result = std::vector<std::shared_ptr<Album>> {};
  result.reserve(m_albums.size());

  ranges::transform(m_albums,
                    std::back_inserter(result),
                    [this](auto& album_element)
                    {
                      if (!album_element.second) {
                        album_element.second = Album::load_album(
                            m_absolute_path / album_element.first,
                            m_save_metadata);
                      }

                      return album_element.second;
                    });

  return result;
}

auto Album::load_album(const std::filesystem::path& album_path,
                       bool save_metadata) -> std::unique_ptr<Album> {
  // Check that the path refers to a directory
  if (!fs::is_directory(album_path)) {
    return {};
  }

  // Check if the path is absolute
  auto absolute_path = fs::absolute(album_path);
  return std::unique_ptr<Album>(
      new Album {std::move(absolute_path), save_metadata});
}
}  // namespace album_architect