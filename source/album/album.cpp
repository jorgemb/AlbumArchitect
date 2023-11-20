//
// Created by jorge on 03/11/2023.
//

#include <ranges>

#include "album.h"

#include "photo.h"

namespace album_architect {

namespace fs = std::filesystem;
namespace ranges = std::ranges;
namespace views = std::ranges::views;

Album::Album(std::filesystem::path absolute_path)
    : m_absolute_path(std::move(absolute_path)) {}
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