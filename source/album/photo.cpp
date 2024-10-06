//
// Created by jorelmb on 18/09/24.
//

#include <optional>
#include <utility>

#include "album/photo.h"

#include <fmt/format.h>
#include <magic_enum.hpp>
#include <opencv2/core/mat.hpp>
#include <spdlog/spdlog.h>

#include "album/image.h"
#include "album/photo_metadata.h"
#include "files/tree.h"

namespace album_architect::album {

using namespace std::string_literals;

auto Photo::load(files::Element file_element) -> std::optional<Photo> {
  auto loaded_image = Image::load(file_element.get_path());
  if (!loaded_image) {
    // Set metadata
    PhotoMetadata::set_photo_state(file_element, PhotoState::error);

    return {};
  }

  // Set metadata
  PhotoMetadata::set_photo_state(file_element, PhotoState::ok);
  return Photo {std::move(file_element), std::move(loaded_image.value())};
}
Photo::Photo(files::Element&& file_element, Image&& image)
    : m_file_element(std::move(file_element))
    , m_image(std::move(image)) {}
auto Photo::get_image() const -> cv::Mat {
  auto out = cv::Mat {};
  m_image.get_image(out);
  return out;
}
auto Photo::get_image_hash(ImageHashAlgorithm algorithm) -> cv::Mat {
  // Check if hash is already stored
  if (auto stored_hash =
          PhotoMetadata::get_stored_hash(m_file_element, algorithm))
  {
    return *stored_hash;
  }

  // Calculate hash and store
  try {
    auto image_hash = m_image.get_image_hash(algorithm);
    PhotoMetadata::store_hash(m_file_element, algorithm, image_hash);
    return image_hash;
  } catch (cv::Exception& e) {
    spdlog::error("Failed to generate hash ({}) for photo: {}. Error: {}",
                  magic_enum::enum_name(algorithm),
                  m_file_element.get_path().string(),
                  e.what());
    return {};
  }
}
auto Photo::is_image_hash_in_cache(ImageHashAlgorithm algorithm) const -> bool {
  return PhotoMetadata::has_hash_stored(m_file_element, algorithm);
}

}  // namespace album_architect::album
