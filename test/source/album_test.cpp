//
// Created by jorge on 09/08/24.
//

#include <filesystem>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <string>

#include <boost/algorithm/string/case_conv.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>
#include <opencv2/core.hpp>
#include <opencv2/img_hash/average_hash.hpp>
#include <opencv2/img_hash/phash.hpp>
#include <spdlog/spdlog.h>

#include "album/image.h"
#include "album/photo.h"
#include "common.h"
#include <magic_enum.hpp>

using namespace album_architect;  // NOLINT(*-build-using-namespace)

// NOLINTNEXTLINE(*-function-cognitive-complexity)
TEST_CASE("Image loading", "[album][image]") {
  // Try loading all images from the directory
  const auto images_dir = resources_dir / "images";

  SECTION("Supported formats") {
    auto loaded_images = std::map<fs::path, std::optional<album::Image>> {};

    for (auto current_file = fs::recursive_directory_iterator(images_dir);
         current_file != fs::recursive_directory_iterator();
         ++current_file)
    {
      // Skip if not file
      if (!current_file->is_regular_file()) {
        continue;
      }

      // Try loading the file
      loaded_images.emplace(current_file->path(),
                            album::Image::load(current_file->path()));
    }

    // Check that images of supported extensions are loaded correctly
    const auto supported_extensions = std::set {".png"s,
                                                ".jpg"s,
                                                ".jpeg"s,
                                                ".bmp"s,
                                                ".tiff"s,
                                                ".tif"s,
                                                ".gif"s,
                                                ".heic"s,
                                                ".cr2"s,
                                                ".psd"s};
    auto unsupported_extensions = std::set<std::string> {};

    for (const auto& [current_path, current_image] : loaded_images) {
      const auto extension =
          boost::to_lower_copy(current_path.extension().string());
      auto position = supported_extensions.find(extension);
      if (position != supported_extensions.end()) {
        // Should be supported
        INFO(fmt::format("Trying image: {} with extension: {}",
                         current_path.string(),
                         extension));
        REQUIRE(current_image);
      } else {
        // Unsupported extension
        unsupported_extensions.insert(extension);
      }
    }

    // .. print unsupported extensions
    auto found_unsupported_extensions =
        std::accumulate(unsupported_extensions.begin(),
                        unsupported_extensions.end(),
                        ""s,
                        [](const auto& lhs, const auto& rhs)
                        { return fmt::format("{}, {}", lhs, rhs); });
    spdlog::info("Found unsupported extensions: {}",
                 found_unsupported_extensions);
  }

  SECTION("Common parameters") {
    // .. test a single image for common parameters
    const auto test_image_path = images_dir / "Home" / "IMG_5515.JPG";
    const auto test_image = album::Image::load(test_image_path);

    constexpr auto expected_width = 3264U;
    constexpr auto expected_height = 2448U;
    constexpr auto expected_channels = 3U;
    REQUIRE(test_image);
    REQUIRE(test_image->get_path() == test_image_path);
    REQUIRE(test_image->get_width() == expected_width);
    REQUIRE(test_image->get_height() == expected_height);
    REQUIRE(test_image->get_channels() == expected_channels);
  }

  SECTION("Metadata") {
    // .. get metadata from an image
    const auto test_image_path = images_dir / "Home" / "IMG_5690.JPG";
    const auto test_image = album::Image::load(test_image_path);

    REQUIRE(test_image);
    const auto metadata = test_image->get_metadata();
    // for (const auto& [k, v] : metadata) {
    //   fmt::println("Metadata: {} -> {}", k, v);
    // }
    REQUIRE(metadata.at("Exif:LensMake"s) == "Apple"s);
    REQUIRE(metadata.at("FNumber"s) == "2.2"s);
    REQUIRE(metadata.at("IPTC:DateCreated"s) == "20150502"s);
  }
}

/// Compares an image with the modified counterpart using each hash type
/// @param image
/// @param modified_image
void compare_hashes(const album::Image& image, cv::InputArray modified_image) {
  // Get original values
  auto original_mat = cv::Mat {};
  REQUIRE(image.get_image(original_mat));
  auto original_average_hash =
      image.get_image_hash(album::ImageHashAlgorithm::average_hash);
  auto original_phash = image.get_image_hash(album::ImageHashAlgorithm::p_hash);
  constexpr auto expected_max_difference = 5.0;

  // Check each hash type
  auto modified_hash = cv::Mat {};
  auto average_hasher = cv::img_hash::AverageHash::create();
  average_hasher->compute(modified_image, modified_hash);
  auto avg_diff = average_hasher->compare(original_average_hash, modified_hash);
  INFO(fmt::format("Average hash difference: {}", avg_diff));
  REQUIRE(avg_diff < expected_max_difference);

  auto p_hasher = cv::img_hash::PHash::create();
  p_hasher->compute(modified_image, modified_hash);
  auto p_diff = p_hasher->compare(original_phash, modified_hash);
  INFO(fmt::format("pHash difference: {}", p_diff));
  REQUIRE(p_diff < expected_max_difference);
}

// NOLINTNEXTLINE(*-function-cognitive-complexity)
TEST_CASE("Hashing", "[album][image][hash]") {
  const auto images_dir = resources_dir / "images";

  SECTION("Basic hashing") {
    struct TestImageHash {
      std::filesystem::path path;
      std::string md5;
      std::string sha256;
    };

    const auto test_images = std::vector<TestImageHash> {
        {images_dir / "Home" / "IMG_5515.JPG",
         "547e4396b2b30a7f52b3b30d008e54b8",
         "b691e9c892b3f09b27390f9c88baa46d72fa9e93b3fd98821b580403826fbeca"},
        {images_dir / "type" / "console.png",
         "ef8d5406cfb4ac1d112a53fbc3a80dd4",
         "6113bee9abb776277ea28d2f78511882ed4411276662790d52620b9ea740239e"},
        {images_dir / "type" / "duke_nukem.bmp",
         "182e8c7a4e4578c7ebe46a32bf3f95f7",
         "6931c69e99d35499d91fd967a1acb59310f06134c9a866fb4944e3378627b8d6"}};

    for (const auto& [path, expected_md5, expected_sha256] : test_images) {
      INFO(fmt::format("Hashing for: {}", path.string()));
      const auto image = album::Image::load(path);
      REQUIRE(image);

      const auto md5 = image->get_hash(album::HashAlgorithm::md5);
      REQUIRE(md5);
      REQUIRE(md5.value() == expected_md5);

      const auto sha256 = image->get_hash(album::HashAlgorithm::sha256);
      REQUIRE(sha256);
      REQUIRE(sha256.value() == expected_sha256);
    }
  }

  SECTION("Image hashing") {
    const auto test_image_dir = images_dir / "size" / "medium_size"
        / "apollo-11-crew-in-raft-before-recovery_9457415403_o.jpg";

    auto test_image = album::Image::load(test_image_dir);
    REQUIRE(test_image);

    auto original_mat = cv::Mat {};
    REQUIRE(test_image->get_image(original_mat));
    auto original_size = cv::Size {static_cast<int>(test_image->get_width()),
                                   static_cast<int>(test_image->get_height())};
    auto modified_mat = cv::Mat {};

    {
      INFO("Downscaling tests");
      cv::resize(
          original_mat, modified_mat, original_size / 4, cv::INTER_LINEAR);
      compare_hashes(test_image.value(), modified_mat);
    }

    {
      INFO("Decoloring tests");
      cv::cvtColor(original_mat, modified_mat, cv::COLOR_BGR2GRAY);
      compare_hashes(test_image.value(), modified_mat);
    }
  }
}

TEST_CASE("Photo Basics", "[album][photo]") {
  const auto images_dir = resources_dir / "images";
  auto file_tree = files::FileTree::build(images_dir);
  REQUIRE(file_tree);

  const auto test_elements = std::vector {
      file_tree->get_element(images_dir / "Home" / "IMG_5515.JPG"),
      file_tree->get_element(images_dir / "type" / "console.png"),
      file_tree->get_element(images_dir / "type" / "duke_nukem.bmp"),
  };

  for(const auto& current: test_elements){
    DYNAMIC_SECTION("Load photo - " << current->get_path().filename().string()){
      REQUIRE(current);

      auto photo = album::Photo::load(*current);
      REQUIRE(photo);

      for(auto [hash_type, hash_name]: magic_enum::enum_entries<album::ImageHashAlgorithm>()) {
        DYNAMIC_SECTION("Testing hash - " << hash_name) {
          REQUIRE_FALSE(album::PhotoMetadata::has_hash_stored(*current, hash_type));
          REQUIRE_FALSE(album::PhotoMetadata::get_stored_hash(*current, hash_type));
          REQUIRE_FALSE(photo->is_image_hash_in_cache(hash_type));

          // Load the photo and load the hash
          auto hash = photo->get_image_hash(hash_type);

          // Check that hash is not only zeroes
          REQUIRE(cv::countNonZero(hash) > 0);

          // Check that hash is now in cache
          REQUIRE(photo->is_image_hash_in_cache(hash_type));
          REQUIRE(album::PhotoMetadata::has_hash_stored(*current, hash_type));
          REQUIRE(album::PhotoMetadata::get_stored_hash(*current, hash_type));
        }
      }
    }
  }
}