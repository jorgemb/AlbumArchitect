#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include <album/photo.h>
#include <boost/algorithm/string.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <glog/logging.h>

using album_architect::Photo;
using namespace std::string_literals;
namespace views = std::ranges::views;
namespace fs = std::filesystem;

/// Holds data about a test image
struct TestImageData {
  std::filesystem::path path;
  uint64_t width, height;
  std::optional<std::string> average_hash, p_hash, color_moment_hash, mh_hash;
};

/// Loads the test image data from a file
/// \return
auto load_test_data() -> std::vector<TestImageData> {
  const auto test_data_path = "image-data.txt"s;
  auto test_data_file = std::ifstream(test_data_path);
  if (!test_data_file) {
    LOG(ERROR) << "Couldn't load test data from: " << test_data_path;
    return {};
  }

  auto result = std::vector<TestImageData> {};

  // Parse the information within the test data
  auto line = std::string();
  while (std::getline(test_data_file, line)) {
    auto elements = std::vector<std::string>();
    boost::split(
        elements,
        line,
        [](auto c) { return c == '\t'; },
        boost::algorithm::token_compress_on);

    // Try getting each of the elements
    auto test_element = TestImageData {};
    if (elements.size() >= 3) {
      test_element.path = fs::path(elements[0]);
      try {
        test_element.width = std::stoull(elements[1]);
        test_element.height = std::stoull(elements[2]);
      } catch (const std::invalid_argument&) {
        LOG(ERROR) << "Couldn't parse line: " << line;
        continue;
      }
    } else {
      LOG(WARNING) << "Line doesn't have enough elements: " << line;
      continue;
    }

    // Try to get the hashes
    for (auto&& hash : elements | views::drop(3)) {
      if (!test_element.average_hash) {
        test_element.average_hash = std::move(hash);
      } else if (!test_element.p_hash) {
        test_element.p_hash = std::move(hash);
      } else if (!test_element.color_moment_hash) {
        test_element.color_moment_hash = std::move(hash);
      } else if (!test_element.mh_hash) {
        test_element.mh_hash = std::move(hash);
      } else {
        LOG(WARNING) << "Extra element on line: " << line;
      }
    }

    result.push_back(std::move(test_element));
  }

  return result;
}

TEST_CASE("Invalid photo", "[album][photo]") {
  // Test invalid photo
  auto invalid_photo = Photo::load("not_a_path.jpeg");
  REQUIRE_FALSE(invalid_photo);

  std::cout << fs::current_path() << '\n';
}

TEST_CASE("Load sample photos", "[album][photo]") {
  // Get test data
  auto test_data = load_test_data();
  REQUIRE_FALSE(test_data.empty());

  // Try getting information from each element
  auto loaded_photos = size_t {};
  for (const auto& test_element : test_data) {
    INFO("Trying file: " << test_element.path);
    auto photo = Photo::load(test_element.path);
    CHECK_FALSE(photo == nullptr);

    if (photo) {
      loaded_photos++;

      // Check each of the hashes
      if (test_element.average_hash) {
        auto test_hash = album_architect::Hash<cv::img_hash::AverageHash> {
            album_architect::from_hex_to_cv<uint8_t>(
                test_element.average_hash.value())};
        auto calculated_hash =
            photo->calculate_average_hash();
        auto difference =
            album_architect::compare_hashes(test_hash, calculated_hash);
        REQUIRE(difference == Catch::Approx(0.0));
      }

      if (test_element.p_hash) {
        auto test_hash = album_architect::Hash<cv::img_hash::PHash> {
            album_architect::from_hex_to_cv<uint8_t>(
                test_element.p_hash.value())};
        auto calculated_hash = photo->calculate_phash();
        auto difference = album_architect::compare_hashes(test_hash, calculated_hash);
        REQUIRE(difference == Catch::Approx(0.0));
      }
    }
  }

  // Check how many photos worked alright
  INFO("Loaded " << test_data.size() << " sample images");
  REQUIRE(loaded_photos == test_data.size());
}

TEST_CASE("Detect faces", "[album][photo]"){
  const auto image_path = fs::path("sample-images") / "Samples" / "HEIC" / "IMG_0378.HEIC";
  auto photo = Photo::load(image_path);

  const auto expected_faces = 6;
  auto faces = photo->get_faces();
  REQUIRE(faces.size() >= expected_faces);

  auto faces_dnn = photo->get_faces_dnn();
  REQUIRE(faces_dnn.size() >= expected_faces);
}

TEST_CASE("Convert from Hex to Mat", "[album][photo]") {
  const auto val1_vector = std::vector<uint8_t> {10, 45, 33, 140};
  auto val1 = cv::Mat {};
  cv::transpose(val1_vector, val1);
  const auto val1_str = "0a2d218c"s;

  auto result1 = album_architect::from_hex_to_cv<uint8_t>(val1_str);
  auto equal = std::equal(
      val1.begin<uint8_t>(), val1.end<uint8_t>(), result1.begin<uint8_t>());
  REQUIRE(equal);
}