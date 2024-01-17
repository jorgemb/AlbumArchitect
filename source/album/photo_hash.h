//
// Created by jorge on 16/01/24.
//

#ifndef ALBUMARCHITECT_PHOTO_HASH_H
#define ALBUMARCHITECT_PHOTO_HASH_H

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

  /// Three-way comparison operator
  /// \param other
  /// \return
  auto operator<=>(const Hash<T>& other) const = default;

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

}  // namespace album_architect

#endif  // ALBUMARCHITECT_PHOTO_HASH_H
