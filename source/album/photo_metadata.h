//
// Created by jorelmb on 29/09/24.
//

#ifndef ALBUMARCHITECT_PHOTO_METADATA_H
#define ALBUMARCHITECT_PHOTO_METADATA_H

#include <optional>
#include <string>

#include <album/image.h>
#include <files/tree.h>
#include <opencv2/core/mat.hpp>

namespace album_architect::album {

/// Represents the last known state of a Photo
enum class PhotoState : std::uint8_t {
  no_info,
  ok,
  error,
};

/// Helps to manage the metadata of a Photo that is stored in the File
/// element
class PhotoMetadata {
public:
  /// Checks if the given hash is stored in metadata of the Photo
  /// @param file_element
  /// @param algorithm
  /// @return
  static auto has_hash_stored(const files::Element& file_element,
                              ImageHashAlgorithm algorithm) -> bool;

  /// Returns the stored hash of the photo, if any
  /// @param file_element
  /// @param algorithm
  /// @return
  static auto get_stored_hash(const files::Element& file_element,
                              ImageHashAlgorithm algorithm)
      -> std::optional<cv::Mat>;

  /// Stores the given hash in the metadata tree
  /// @param file_element
  /// @param algorithm
  /// @param hash
  static void store_hash(files::Element& file_element,
                         ImageHashAlgorithm algorithm,
                         cv::Mat hash);

  /// Returns the current PhotoState for the file element
  /// @param file_element
  /// @return
  static auto get_photo_state(const files::Element& file_element) -> PhotoState;

  /// Sets a new PhotoState for the file element
  /// @param file_element
  /// @param state
  static void set_photo_state(files::Element& file_element, PhotoState state);

private:
  /// Returns the hash key for the given hash algorithm
  /// \param algorithm Algorithm to check
  /// \return String with the hash value
  static auto get_hash_key(ImageHashAlgorithm algorithm) -> std::string;

  /// Returns the hash key for the PhotoState metadata
  /// @return
  static auto get_photo_state_key() -> std::string;
};

}  // namespace album_architect::album

#endif  // ALBUMARCHITECT_PHOTO_METADATA_H
