//
// Created by jorge on 11/10/2023.
//

#ifndef ALBUMARCHITECT_PHOTO_H
#define ALBUMARCHITECT_PHOTO_H

#include <filesystem>
#include <optional>
#include <ranges>
#include <limits>

#include <boost/algorithm/hex.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/img_hash.hpp>
#include <openimageio/imagebuf.h>

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

  /// Calculates a hash of the given type
  /// \tparam T
  /// \return
  template<class T>
    requires std::is_base_of_v<cv::img_hash::ImgHashBase, T>
  auto calculate_hash() -> Hash<T> {
    // Loads the image
    load_opencv();

    // Create hash
    auto hasher = T::create();
    auto result = cv::Mat {};
    hasher->compute(m_image_cv, result);

    return Hash<T> {result};
  }

  /// Destructor
  virtual ~Photo() = default;

  // Constructors and assignment
  Photo(const Photo& other) = default;
  Photo(Photo&& other) noexcept = default;
  auto operator=(const Photo& other) -> Photo& = default;
  auto operator=(Photo&& other) noexcept -> Photo& = default;

private:
  /// Path of the photo
  std::filesystem::path m_path;

  /// Internal photo
  OIIO::ImageBuf m_image;
  cv::Mat m_image_cv;

  /// Default constructor
  Photo(std::filesystem::path path, OIIO::ImageBuf image);

  /// Loads OpenCV image on demand
  void load_opencv();
};

/// Tries to convert a hexadecimal value to cv::Mat
template<class T>
auto from_hex_to_cv(std::string_view hex_string) -> cv::Mat {
  // Convert to values
  std::vector<T> values;
  boost::algorithm::unhex(hex_string, std::back_insert_iterator(values));


  auto result = cv::Mat{};
  cv::transpose(cv::Mat(values, true), result);
  return result;
}

}  // namespace album_architect

#endif  // ALBUMARCHITECT_PHOTO_H
