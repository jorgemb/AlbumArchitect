//
// Created by jorelmb on 18/09/24.
//

#ifndef ALBUMARCHITECT_PHOTO_H
#define ALBUMARCHITECT_PHOTO_H

#include <optional>
#include <string>

#include <opencv2/core/mat.hpp>

#include "album/image.h"
#include "files/tree.h"

namespace album_architect::album {

/// Helps managing the metadata of a Photo that is stored in the File
/// element
class PhotoMetadata {
public:
  /// Returns the hash key for the given hash algorithm
  /// \param algorithm Algorithm to check
  /// \return String with the hash value
  static auto get_hash_key(ImageHashAlgorithm algorithm) -> std::string;

  /// Checks if the given hash is stored in metadata of the Photo
  static auto has_hash_stored(const files::Element& file_element,
                              ImageHashAlgorithm algorithm) -> bool;

  /// Returns the stored hash of the photo, if any
  static auto get_stored_hash(const files::Element& file_element,
                              ImageHashAlgorithm algorithm) -> std::optional<cv::Mat>;
};

/// A Photo represents the union of a file with an Image.
class Photo {
public:
  /// Loads the photo at the specified file element
  /// \param file_element Element with the path to the photo
  /// \return Photo or null if there was an error
  static auto load(files::Element file_element) -> std::optional<Photo>;

  /// Returns a cv::Mat with the image information.
  /// \return cv::Mat with image information
  auto get_image() -> cv::Mat;

  /// Returns a cv::Mat with the specified image hash. The value is
  /// cached and stored for future reference.
  /// \return cv::Mat with hash
  auto get_image_hash(ImageHashAlgorithm algorithm) -> cv::Mat;

  /// Returns True if the given hash is stored in the cache.
  /// \return true if hash in cache.
  auto is_image_hash_in_cache(ImageHashAlgorithm algorithm) const -> bool;

private:
  /// Default constructor.
  /// \param file_element Element that represents the file information
  /// \param image Actual image
  Photo(files::Element&& file_element, Image&& image);

  files::Element m_file_element;
  Image m_image;
};

}  // namespace album_architect::album

#endif  // ALBUMARCHITECT_PHOTO_H
