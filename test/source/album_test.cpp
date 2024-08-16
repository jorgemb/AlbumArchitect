//
// Created by jorge on 09/08/24.
//

#include <map>
#include <numeric>
#include <optional>
#include <set>

#include <boost/algorithm/string/case_conv.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "album/image.h"
#include "common.h"

using namespace albumarchitect;  // NOLINT(*-build-using-namespace)

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

  SECTION("Hashing") {
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
}