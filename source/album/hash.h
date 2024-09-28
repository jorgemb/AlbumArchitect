#ifndef HASH_H
#define HASH_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <opencv2/core/mat.hpp>

namespace album_architect::hash {

/// Supports the creation of several hashes out of files or data
class Hash {
public:
  /// Calculates the hash of a given file using MD5
  /// @param path
  /// @return
  static auto calculate_md5(const std::filesystem::path& path)
      -> std::optional<std::string>;

  /// Calculates the hash of a given file using SHA256
  /// @param path
  /// @return
  static auto calculate_sha256(const std::filesystem::path& path)
      -> std::optional<std::string>;

  /// Calculates the average hash of the given input
  /// @param input
  /// @return
  static auto calculate_average_hash(cv::InputArray input) -> cv::Mat;

  /// Calculates the pHash of the given input
  /// @param input
  /// @return
  static auto calculate_p_hash(cv::InputArray input) -> cv::Mat;
};

}  // namespace album_architect::hash

#endif  // HASH_H
