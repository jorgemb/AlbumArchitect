#ifndef HASH_H
#define HASH_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace albumarchitect::hash {

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
};

}  // namespace albumarchitect::hash

#endif  // HASH_H