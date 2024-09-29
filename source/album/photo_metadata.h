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

/// Helps managing the metadata of a Photo that is stored in the File
/// element
class PhotoMetadata {
public:
  /// Checks if the given hash is stored in metadata of the Photo
  static auto has_hash_stored(const files::Element& file_element,
                              ImageHashAlgorithm algorithm) -> bool;

  /// Returns the stored hash of the photo, if any
  static auto get_stored_hash(const files::Element& file_element,
                              ImageHashAlgorithm algorithm)
      -> std::optional<cv::Mat>;

  /// Stores the given hash in the metadata tree
  static void store_hash(files::Element& file_element,
                         ImageHashAlgorithm algorithm,
                         cv::Mat hash);

private:
  /// Returns the hash key for the given hash algorithm
  /// \param algorithm Algorithm to check
  /// \return String with the hash value
  static auto get_hash_key(ImageHashAlgorithm algorithm) -> std::string;
};

}  // namespace album_architect::album

#endif  // ALBUMARCHITECT_PHOTO_METADATA_H
