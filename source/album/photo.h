//
// Created by jorge on 11/10/2023.
//

#ifndef ALBUMARCHITECT_PHOTO_H
#define ALBUMARCHITECT_PHOTO_H

#include <filesystem>
#include <ostream>
#include <ranges>

#include <OpenImageIO/imagebuf.h>
#include <boost/algorithm/hex.hpp>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/vector.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/img_hash.hpp>
#include <support/serialize/cvmat.h>

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

  /// Serialization function for hash
  /// \tparam Archive
  /// \param archive
  template<class Archive>
  void serialize(Archive& archive) {
    archive(hash);
  }
};

using AverageHash = Hash<cv::img_hash::AverageHash>;
using PHash = Hash<cv::img_hash::PHash>;
using ColorMomentHash = Hash<cv::img_hash::ColorMomentHash>;
using MarrHildrethHash = Hash<cv::img_hash::MarrHildrethHash>;

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

  /* Serialization */
  template<class Archive>
  void serialize(Archive& archive) {
    archive(CEREAL_NVP(creation_time),
            CEREAL_NVP(date_time),
            CEREAL_NVP(description),
            CEREAL_NVP(keywords));
  }
};

/// Represents a photo that can be loaded and processed.
class Photo final {
public:
  auto get_path() const -> const std::filesystem::path&;
  auto get_width() const -> int64_t;
  auto get_height() const -> int64_t;

  // Hashes
  auto calculate_average_hash() -> AverageHash;
  auto calculate_phash() -> PHash;
  auto calculate_color_moment_hash() -> ColorMomentHash;
  auto calculate_marr_hildreth_hash() -> MarrHildrethHash;

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

  /// \brief Returns true if the Photo is loaded and working
  /// \return
  auto is_ok() const -> bool;

  /// \brief Default constructor
  Photo() = default;

  /// \brief Constructor with path
  /// \param path
  explicit Photo(const std::filesystem::path& path);

  /// Destructor
  virtual ~Photo() = default;

  /// Get the internal opencv2 representation
  auto get_cv_mat() -> const cv::Mat&;

  // Constructors and assignment
  Photo(const Photo& other) = delete;
  auto operator=(const Photo& other) -> Photo& = delete;

  Photo(Photo&& other) noexcept = default;
  auto operator=(Photo&& other) noexcept -> Photo& = default;

private:
  /// \brief Loads the metadata from the photo file
  auto load_metadata() -> bool;

  /// Path of the photo
  std::filesystem::path m_path;

  /// Internal data
  OIIO::ImageBuf m_image;
  cv::Mat m_image_cv;

  /// Hash cache
  std::optional<AverageHash> m_average_hash;
  std::optional<PHash> m_phash;
  std::optional<ColorMomentHash> m_color_moment_hash;
  std::optional<MarrHildrethHash> m_marr_hildreth_hash;

  /// Standard metadata
  PhotoMetadata m_metadata;

  /// Default constructor
  Photo(std::filesystem::path path,
        OIIO::ImageBuf&& image,
        PhotoMetadata&& metadata);

  /// Loads OpenCV image on demand
  void load_opencv();

  /* SERIALIZATION */
  friend class cereal::access;

  /// \brief Save function for serialization
  /// \tparam Archive
  /// \param archive
  template<class Archive>
  void save(Archive& archive) const {
    archive(m_path.string(), m_metadata, m_average_hash, m_phash, m_color_moment_hash, m_marr_hildreth_hash);
  }

  /// \brief Load function for serialization
  /// \tparam Archive
  /// \param archive
  template<class Archive>
  void load(Archive& archive) {
    // Load values
    auto path = std::string {};
    archive(path, m_metadata, m_average_hash, m_phash, m_color_moment_hash, m_marr_hildreth_hash);
    m_path = path;

    // Load the image buffer
    m_image = OIIO::ImageBuf(path);
  }
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
