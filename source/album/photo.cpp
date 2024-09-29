//
// Created by jorelmb on 18/09/24.
//

#include <optional>
#include <utility>
#include <variant>

#include "album/photo.h"

#include <fmt/format.h>
#include <magic_enum.hpp>
#include <opencv2/core/mat.hpp>

#include "album/image.h"
#include "album/photo_metadata.h"
#include "files/tree.h"

namespace album_architect::album {

using namespace std::string_literals;

auto Photo::load(files::Element file_element) -> std::optional<Photo> {
  auto loaded_image = Image::load(file_element.get_path());
  if (!loaded_image) {
    return {};
  }

  return Photo {std::move(file_element), std::move(loaded_image.value())};
}
Photo::Photo(files::Element&& file_element, Image&& image)
    : m_file_element(std::move(file_element))
    , m_image(std::move(image)) {}
auto Photo::get_image() -> cv::Mat {
  auto out = cv::Mat {};
  m_image.get_image(out);
  return out;
}
auto Photo::get_image_hash(ImageHashAlgorithm algorithm) -> cv::Mat {
  // Check if hash is already stored
  auto stored_hash = PhotoMetadata::get_stored_hash(m_file_element, algorithm);
  if(stored_hash){
    return *stored_hash;
  }

  // Calculate hash and store
  auto image_hash = m_image.get_image_hash(algorithm);
  PhotoMetadata::store_hash(m_file_element, algorithm, image_hash);
  return image_hash;
}
auto Photo::is_image_hash_in_cache(ImageHashAlgorithm algorithm) const -> bool {
  return PhotoMetadata::has_hash_stored(m_file_element, algorithm);
}

}  // namespace album_architect::album
