//
// Created by jorelmb on 18/09/24.
//

#include <optional>
#include <utility>

#include "album/photo.h"

#include <magic_enum/magic_enum.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <spdlog/spdlog.h>

#include "album/image.h"
#include "album/photo_metadata.h"
#include "files/tree.h"

namespace album_architect::album {

using namespace std::string_literals;

auto Photo::load(files::Element file_element) -> std::optional<Photo> {
  // Check if the photo is known to have errors
  if (PhotoMetadata::get_photo_state(file_element) == PhotoState::error) {
    return {};
  }

  // Check if the photo would be possible to be loaded
  if (!Image::check_path_is_image(file_element.get_path())) {
    PhotoMetadata::set_photo_state(file_element, PhotoState::error);
    spdlog::error("File {} does not point to a valid photo.",
                  file_element.get_path().string());
    return {};
  }

  return Photo {std::move(file_element)};
}
Photo::Photo(files::Element&& file_element)
    : m_file_element(std::move(file_element)) {}
auto Photo::load_image() -> bool {
  if (m_image) {
    return true;
  }

  // Try loading the image
  auto loaded_image = Image::load(m_file_element.get_path());
  if (!loaded_image) {
    // Set metadata to error
    PhotoMetadata::set_photo_state(m_file_element, PhotoState::error);
    return false;
  }

  PhotoMetadata::set_photo_state(m_file_element, PhotoState::ok);
  m_image = std::move(loaded_image);

  return true;
}
auto Photo::get_image() -> std::optional<cv::Mat> {
  // Check if image is loaded
  if (!load_image() || !m_image) {
    return {};
  }

  // Get the image data
  auto out = cv::Mat {};
  m_image->get_image(out);

  return out;
}
auto Photo::get_file_element() const -> files::Element {
  return m_file_element;
}
auto Photo::get_image_hash(ImageHashAlgorithm algorithm)
    -> std::optional<cv::Mat> {
  // Check if hash is already stored
  if (auto stored_hash =
          PhotoMetadata::get_stored_hash(m_file_element, algorithm))
  {
    return stored_hash;
  }

  // Check if image is loaded
  if (!load_image() || !m_image) {
    return {};
  }

  // Calculate hash and store
  try {
    auto image_hash = m_image->get_image_hash(algorithm);
    PhotoMetadata::store_hash(m_file_element, algorithm, image_hash);
    return image_hash;
  } catch (cv::Exception& e) {
    spdlog::error("Failed to generate hash ({}) for photo: {}. Error: {}",
                  magic_enum::enum_name(algorithm),
                  m_file_element.get_path().string(),
                  e.what());
    PhotoMetadata::set_photo_state(m_file_element, PhotoState::error);
    return {};
  }
}
auto Photo::is_image_hash_in_cache(ImageHashAlgorithm algorithm) const -> bool {
  return PhotoMetadata::has_hash_stored(m_file_element, algorithm);
}

}  // namespace album_architect::album
