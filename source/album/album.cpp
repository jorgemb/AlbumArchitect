//
// Created by jorge on 03/11/2023.
//

#include <ranges>
#include <set>
#include <glog/logging.h>

#include "album.h"

#include "photo.h"

namespace album_architect {

namespace fs = std::filesystem;
namespace ranges = std::ranges;
namespace views = std::ranges::views;

Album::Album(std::filesystem::path absolute_path)
    : m_absolute_path(std::move(absolute_path)) {}
auto Album::update_album() -> void {
  DLOG(INFO) << "Updating album at: " << m_absolute_path;

  m_files.clear();

  auto processed_albums = std::set<std::string_view>{};
  auto processed_photos = std::set<std::string_view>{};

  // Get all files under the directory
  for(const auto &entry: fs::directory_iterator(m_absolute_path)) {
    // Add current files to files cache
    m_files.push_back(entry.path().filename().string());
    auto &current_file = m_files.back();

    // Check if it is a photo or album
    if(entry.is_directory()) {
      // .. add Album
      processed_albums.insert(current_file);
      if(!m_albums.contains(current_file)) {
        // New Album found
        DLOG(INFO) << "--Album found at " << current_file;
        m_albums[current_file] = std::move(Album::load_album(entry.path()));
      }
    } else {
      // .. add File, probable Photo
      if(!m_photos.contains(current_file)) {
        // Try to load as Photo
        if (auto photo = Photo::load(entry.path())) {
          DLOG(INFO) << "--Photo found at " << current_file;
          processed_photos.insert(current_file);
          m_photos[current_file] = std::move(photo);
        }
      } else {
        processed_photos.insert(current_file);
      }
    }
  }

  // Check if there are photos / albums that are not in the list
  for(auto current_album = m_albums.begin(); current_album != m_albums.end(); /* no increment */) {
    if(!processed_albums.contains(current_album->first)) {
      // Delete album
      DLOG(INFO) << "--Removed album at " << current_album->first;
      current_album = m_albums.erase(current_album);
    } else {
      ++current_album;
    }
  }

  for(auto current_photo = m_photos.begin(); current_photo != m_photos.end(); /* no increment */) {
    if(!processed_photos.contains(current_photo->first)) {
      // Delete photo
      DLOG(INFO) << "--Removed photo at " << current_photo->first;
      current_photo = m_photos.erase(current_photo);
    } else {
      ++current_photo;
    }
  }
}
auto Album::get_files() const -> std::vector<std::filesystem::path> {
  auto result = std::vector<std::filesystem::path> {};
  ranges::copy_if(
      fs::directory_iterator(m_absolute_path),
      std::back_inserter(result),
      [](const auto& entry) { return fs::is_regular_file(entry); },
      [](const auto& entry) { return entry.path(); });

  return result;
}
auto Album::get_photos() const -> std::vector<std::shared_ptr<Photo>> {
  auto result = std::vector<std::shared_ptr<Photo>> {};
  for (const auto& file_path : get_files()) {
    auto photo = Photo::load(file_path);
    if (photo) {
      result.push_back(std::move(photo));
    }
  }

  return result;
}
auto Album::get_albums() const -> std::vector<std::shared_ptr<Album>> {
  // Load all directories from this Album
  auto result = std::vector<std::shared_ptr<Album>> {};

  auto is_directory = [](const auto& entry) { return fs::is_directory(entry); };
  ranges::transform(
      fs::directory_iterator(m_absolute_path) | views::filter(is_directory),
      std::back_inserter(result),
      [](const auto& entry) { return load_album(entry.path()); });

  return result;
}

auto Album::load_album(const std::filesystem::path& album_path)
    -> std::unique_ptr<Album> {
  // Check that the path refers to a directory
  if (!fs::is_directory(album_path)) {
    return {};
  }

  // Check if the path is absolute
  auto absolute_path = fs::absolute(album_path);
  return std::unique_ptr<Album>(new Album {std::move(absolute_path)});
}
}  // namespace album_architect