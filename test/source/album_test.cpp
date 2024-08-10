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

#include "album/image.h"
#include "common.h"

using namespace albumarchitect;  // NOLINT(*-build-using-namespace)

TEST_CASE("Image loading", "[album][image]") {
  // Try loading all images from the directory
  const auto images_dir = resources_dir / "images";
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
  fmt::println("Found unsupported extensions: {}",
               found_unsupported_extensions);
}