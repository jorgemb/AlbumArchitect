//
// Created by jorge on 11/10/2023.
//

#ifndef ALBUMARCHITECT_PHOTO_H
#define ALBUMARCHITECT_PHOTO_H

#include <filesystem>
#include <limits>
#include <optional>
#include <ostream>
#include <ranges>

#include <OpenImageIO/imagebuf.h>
#include <boost/algorithm/hex.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/img_hash.hpp>

namespace album_architect {

/// Concept to represent hashers that derive from ImgHashBase (OpenCV)
/// \tparam T
template<class T>
concept ImgHasher = std::is_base_of_v<cv::img_hash::ImgHashBase, T>;

/// Represents the Hash calculated by a given Image Hasher.
/// \tparam T
template<ImgHasher T>
struct Hash {
  cv::Mat hash;
};

/// Compares two hashes, returning a value of how close they are
/// \tparam T
/// \param lhs
/// \param rhs
/// \return
template<ImgHasher T>
auto compare_hashes(const Hash<T>& lhs, const Hash<T>& rhs) -> double {
  auto hasher = T::create();
  return hasher->compare(lhs.hash, rhs.hash);
}

/// Represents a text element that was identified in photo OCR
struct TextElement {
  cv::Rect2f rect;
  std::string text;
  float confidence;
};

/// Represents the basic metadata that should be shared for all images
struct PhotoMetadata {
  std::string creation_time;
  std::string date_time;
  std::string description;
  std::vector<std::string> keywords;

  auto operator<=>(const PhotoMetadata& other) const = default;

  /// Stream Output operator for PhotoMetadata
  /// \param os
  /// \param metadata
  /// \return
  friend auto operator<<(std::ostream& ostream, const PhotoMetadata& metadata)
      -> std::ostream&;
};

/// Represents a photo that can be loaded and processed.
class Photo {
public:
  /// Tries to create a new photo from the given path. Returns
  /// \param path
  /// \return
  static auto load(const std::filesystem::path& path) -> std::unique_ptr<Photo>;

  auto get_path() const -> const std::filesystem::path&;
  auto get_width() const -> int64_t;
  auto get_height() const -> int64_t;

  // Hashes
  auto calculate_average_hash() -> Hash<cv::img_hash::AverageHash>;
  auto calculate_phash() -> Hash<cv::img_hash::PHash>;
  auto calculate_color_moment_hash() -> Hash<cv::img_hash::ColorMomentHash>;
  auto calculate_marr_hildreth_hash() -> Hash<cv::img_hash::MarrHildrethHash>;

  /// Tries to detect the faces from within the photo
  /// \return
  auto get_faces() -> std::vector<cv::Rect2f>;

  /// Tries to detect faces from within the photo using DNN
  /// \return List of rects
  auto get_faces_dnn() -> std::vector<cv::Rect2f>;

  /// Tries to perform OCR on the text, and returns a list
  /// of found elements.
  /// \return
  auto get_text_ocr() -> std::vector<TextElement>;

  /// Returns a view of the metadata
  /// \return
  auto get_metadata() const -> const PhotoMetadata&;

  /// Destructor
  virtual ~Photo() = default;

#ifdef _DEBUG
  /// Get the internal opencv2 representation
  auto get_cv_mat() -> const cv::Mat&;
#endif

  // Constructors and assignment
  Photo(const Photo& other) = delete;
  auto operator=(const Photo& other) -> Photo& = delete;

  Photo(Photo&& other) noexcept = default;
  auto operator=(Photo&& other) noexcept -> Photo& = default;

private:
  /// Path of the photo
  std::filesystem::path m_path;

  /// Internal data
  OIIO::ImageBuf m_image;
  cv::Mat m_image_cv;

  /// Standard metadata
  PhotoMetadata m_metadata;

  /// Default constructor
  Photo(std::filesystem::path path,
        OIIO::ImageBuf&& image,
        PhotoMetadata&& metadata);

  /// Loads OpenCV image on demand
  void load_opencv();
};

/// Tries to convert a hexadecimal value to cv::Mat
template<class T>
auto from_hex_to_cv(std::string_view hex_string) -> cv::Mat {
  // Convert to values
  std::vector<T> values;
  boost::algorithm::unhex(hex_string, std::back_insert_iterator(values));

  auto result = cv::Mat {};
  cv::transpose(cv::Mat(values, true), result);
  return result;
}

}  // namespace album_architect

#endif  // ALBUMARCHITECT_PHOTO_H
