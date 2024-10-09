//
// Created by jorelmb on 29/09/24.
//

#include "photo_metadata.h"

#include <fmt/format.h>
#include <magic_enum.hpp>

namespace album_architect::album {
using namespace std::string_literals;

auto PhotoMetadata::get_hash_key(ImageHashAlgorithm algorithm) -> std::string {
  return fmt::format("_HASH_{}_", magic_enum::enum_name(algorithm));
}
auto PhotoMetadata::has_hash_stored(const files::Element& file_element,
                                    ImageHashAlgorithm algorithm) -> bool {
  // TODO: Add a function to only check if exists, so a copy is avoided on
  // get_metadata
  const auto hash_value =
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
void PhotoMetadata::store_hash(files::Element& file_element,
                               ImageHashAlgorithm algorithm,
                               cv::Mat hash) {
  const auto hash_key = PhotoMetadata::get_hash_key(algorithm);
  file_element.set_metadata(hash_key, hash);
}
auto PhotoMetadata::get_photo_state(const files::Element& file_element)
    -> PhotoState {
  const auto state_key = get_photo_state_key();
  const auto stored_state = file_element.get_metadata(state_key);
  if (!stored_state || !std::holds_alternative<std::string>(*stored_state)) {
    return PhotoState::no_info;
  }

  // Try to parse stored state
  const auto state =
      magic_enum::enum_cast<PhotoState>(std::get<std::string>(*stored_state));
  if (!state.has_value()) {
    return PhotoState::no_info;
  }

  return state.value();
}
void PhotoMetadata::set_photo_state(files::Element& file_element,
                                    PhotoState state) {
  const auto state_key = get_photo_state_key();
  file_element.set_metadata(state_key,
                            std::string {magic_enum::enum_name(state)});
}
auto PhotoMetadata::get_photo_state_key() -> std::string {
  return "_PHOTO_STATE_"s;
}
}  // namespace album_architect::album