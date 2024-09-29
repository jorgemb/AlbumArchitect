//
// Created by jorelmb on 29/09/24.
//

#include "photo_metadata.h"

#include <fmt/format.h>
#include <magic_enum.hpp>

namespace album_architect {
namespace album {
auto PhotoMetadata::get_hash_key(ImageHashAlgorithm algorithm) -> std::string {
  return fmt::format("HASH_{}", magic_enum::enum_name(algorithm));
}
auto PhotoMetadata::has_hash_stored(const files::Element& file_element,
                                    ImageHashAlgorithm algorithm) -> bool {
  // TODO: Add a function to only check if exists, so a copy is avoided on
  // get_metadata
  auto hash_value =
      file_element.get_metadata(PhotoMetadata::get_hash_key(algorithm));
  return hash_value && std::holds_alternative<cv::Mat>(*hash_value);
}
auto PhotoMetadata::get_stored_hash(const files::Element& file_element,
                                    ImageHashAlgorithm algorithm)
    -> std::optional<cv::Mat> {
  auto hash_value =
      file_element.get_metadata(PhotoMetadata::get_hash_key(algorithm));
  if (!hash_value || !std::holds_alternative<cv::Mat>(*hash_value)) {
    return {};
  }

  return {std::move(std::get<cv::Mat>(*hash_value))};
}
}  // namespace album
}  // namespace album_architect