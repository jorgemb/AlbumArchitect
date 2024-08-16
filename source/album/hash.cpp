//
// Created by jorge on 16/08/24.
//

#include <array>
#include <fstream>
#include <iostream>

#include "hash.h"

#include <hash-library/md5.h>
#include <spdlog/spdlog.h>

namespace albumarchitect::hash {

/// Size of the buffer for image loading
constexpr auto image_load_buffer_size = std::size_t {4096U};

auto Hash::calculate_md5(const std::filesystem::path& path)
    -> std::optional<std::string> {
  // Load image
  auto image_file = std::ifstream(path, std::ios::binary);
  if (!image_file) {
    spdlog::error("Couldn´t load image file in {}", path.string());
    return {};
  }

  // Read data from image file into hasher
  auto buffer = std::array<std::ifstream::char_type, image_load_buffer_size> {};
  auto hash = MD5 {};
  while (image_file) {
    image_file.read(buffer.data(), image_load_buffer_size);
    const auto n_read = image_file.gcount();
    hash.add(buffer.data(), static_cast<std::size_t>(n_read));
  }

  return hash.getHash();
}
}  // namespace albumarchitect::hash