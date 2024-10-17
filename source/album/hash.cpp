//
// Created by jorge on 16/08/24.
//

#include <array>
#include <fstream>
#include <iostream>

#include "hash.h"

#include <hash-library/md5.h>
#include <hash-library/sha256.h>
#include <opencv2/img_hash/average_hash.hpp>
#include <opencv2/img_hash/phash.hpp>
#include <spdlog/spdlog.h>

namespace album_architect::hash {

/// Size of the buffer for image loading
constexpr auto image_load_buffer_size = std::size_t {4096U};

/// General function for calculating a hash
/// @tparam T
/// @param path
/// @return
template<class T>
auto calculate_hash(const std::filesystem::path& path)
    -> std::optional<std::string> {
  // Load image
  auto image_file = std::ifstream(path, std::ios::binary);
  if (!image_file) {
    spdlog::error("Couldn't load image file in {}", path.string());
    return {};
  }

  // Read data from image file into hasher
  auto buffer = std::array<std::ifstream::char_type, image_load_buffer_size> {};
  auto hash = T {};
  while (image_file) {
    image_file.read(buffer.data(), image_load_buffer_size);
    const auto n_read = image_file.gcount();
    hash.add(buffer.data(), static_cast<std::size_t>(n_read));
  }

  return hash.getHash();
}

auto Hash::calculate_md5(const std::filesystem::path& path)
    -> std::optional<std::string> {
  return calculate_hash<MD5>(path);
}
auto Hash::calculate_sha256(const std::filesystem::path& path)
    -> std::optional<std::string> {
  return calculate_hash<SHA256>(path);
}
auto Hash::calculate_average_hash(const cv::Mat& input) -> cv::Mat {
  auto hasher = cv::img_hash::AverageHash::create();

  auto output = cv::Mat {};
  hasher->compute(input, output);
  return output;
}
auto Hash::calculate_p_hash(const cv::Mat& input) -> cv::Mat {
  auto hasher = cv::img_hash::PHash::create();

  auto output = cv::Mat {};
  hasher->compute(input, output);
  return output;
}
}  // namespace album_architect::hash