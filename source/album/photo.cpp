//
// Created by jorelmb on 18/09/24.
//

#include <optional>
#include <utility>
#include <variant>
#include <fmt/format.h>

#include "album/photo.h"

#include <magic_enum.hpp>
#include <opencv2/core/mat.hpp>

#include "album/image.h"
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
  auto hash_store_key = PhotoMetadata::get_hash_key(algorithm);

  // Check if hash is stored
  auto hash_value = m_file_element.get_metadata(hash_store_key);
  if (hash_value && std::holds_alternative<cv::Mat>(*hash_value)) {
    return std::get<cv::Mat>(*hash_value);
  }

  // Calculate hash and store
  auto image_hash = m_image.get_image_hash(algorithm);
  m_file_element.set_metadata(hash_store_key, image_hash);
  return image_hash;
}
auto Photo::is_image_hash_in_cache(ImageHashAlgorithm algorithm) -> bool {
  // TODO: Add a function to only check if exists, so a copy is avoided on get_metadata
  auto hash_value = m_file_element.get_metadata(PhotoMetadata::get_hash_key(algorithm));
  return hash_value && std::holds_alternative<cv::Mat>(*hash_value);
}
auto PhotoMetadata::get_hash_key(ImageHashAlgorithm algorithm) -> std::string {
  return fmt::format("HASH_{}", magic_enum::enum_name(algorithm));
  return std::string();
}
}  // namespace album_architect::album
