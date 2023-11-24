#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include <album/album.h>
#include <album/photo.h>
#include <album/util.h>
#include <boost/algorithm/string.hpp>
#include <boost/range/combine.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <glog/logging.h>

using album_architect::Photo;
using namespace std::string_literals;
namespace views = std::ranges::views;
namespace ranges = std::ranges;
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
  auto invalid_photo = Photo("not_a_path.jpeg");
  REQUIRE_FALSE(invalid_photo.is_ok());

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
    auto photo = Photo(test_element.path);
    CHECK(photo.is_ok());

    if (photo.is_ok()) {
      loaded_photos++;

      // Check each of the hashes
      if (test_element.average_hash) {
        auto test_hash = album_architect::Hash<cv::img_hash::AverageHash> {
            album_architect::from_hex_to_cv<uint8_t>(
                test_element.average_hash.value())};
        auto calculated_hash = photo.calculate_average_hash();
        auto difference =
            album_architect::compare_hashes(test_hash, calculated_hash);
        REQUIRE(difference == Catch::Approx(0.0));
      }

      if (test_element.p_hash) {
        auto test_hash = album_architect::Hash<cv::img_hash::PHash> {
            album_architect::from_hex_to_cv<uint8_t>(
                test_element.p_hash.value())};
        auto calculated_hash = photo.calculate_phash();
        auto difference =
            album_architect::compare_hashes(test_hash, calculated_hash);
        REQUIRE(difference == Catch::Approx(0.0));
      }
    }
  }

  // Check how many photos worked alright
  INFO("Loaded " << test_data.size() << " sample images");
  REQUIRE(loaded_photos == test_data.size());
}

TEST_CASE("Detect faces", "[album][photo][high-load]") {
  const auto image_path =
      fs::path("sample-images") / "Samples" / "HEIC" / "IMG_0378.HEIC";
  auto photo = Photo(image_path);

  const auto expected_faces = 6;
  auto faces = photo.get_faces();
  REQUIRE(faces.size() >= expected_faces);

  auto faces_dnn = photo.get_faces_dnn();
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

TEST_CASE("Retrieve image metadata", "[album][photo][metadata]") {
  auto images = std::vector<std::string> {
      "sample-images/Samples/HEIC/IMG_0378.HEIC"s,
      "sample-images/Samples/BMP/error/33A3F6D37FE162E4.bmp"s,
      "sample-images/Samples/PNG/gamma_error/error file.png"s,
      "sample-images/Samples/JPG/earth_12k.jpg"s};

  auto metadata = std::vector<album_architect::PhotoMetadata> {
      album_architect::PhotoMetadata {
          "2017:10:27 20:37:16", "2017:10:27 20:37:16", "", {}},
      album_architect::PhotoMetadata {},
      album_architect::PhotoMetadata {},
      album_architect::PhotoMetadata {""s,
                                      ""s,
                                      R"(Satellite: Suomi NPP
Sensor: VIIRS
Date: 3 February 2012
Description: Perspective view of North Atlantic< Europe, Africa
Red channel: Band 5 (662-682 nm)
Green channel: Band 4 (545-565 nm)
Blue channel: Band 2 (436-454 nm)
Projection: Near-sided perspective from
            12742 kilometers above 45 North by 0 East
Author: Norman Kuring
Created: 2012-02-21T14:00:00Z
)"},
  };

  // Iterate through all the photos
  for (auto const [image_path, expected_metadata] :
       boost::combine(images, metadata))
  {
    INFO("Current image: " << image_path);
    auto current_image = album_architect::Photo(image_path);
    auto current_metadata = current_image.get_metadata();

    REQUIRE(current_metadata == expected_metadata);
  }
}

TEST_CASE("Load and retrieve Albums", "[album][albums]") {
  using album_architect::Album;

  // Loads test data
  const auto test_path = fs::path("sample-images") / "Samples"s;

  // Loads a non-existent album
  auto non_existent_album = Album::load_album("not/existent/path");
  REQUIRE_FALSE(non_existent_album);

  // Tries to load an album from the Path
  auto top_level_album = Album::load_album(test_path);
  REQUIRE(top_level_album);
  REQUIRE(top_level_album->get_absolute_path()
          == fs::current_path() / test_path);

  // There should be no images and no files at the top level
  auto top_photos = top_level_album->get_photos();
  REQUIRE(top_photos.empty());
  auto top_files = top_level_album->get_files();
  REQUIRE(top_files.empty());

  // There should be an album per folder
  auto top_albums = top_level_album->get_albums();
  constexpr auto expected_top_albums = 27;
  REQUIRE(top_albums.size() == expected_top_albums);

  // Get one of the albums
  auto jpeg_album_iter = ranges::find_if(
      top_albums, [](const auto& album) { return album->name() == "JPG"s; });
  REQUIRE(jpeg_album_iter != top_albums.end());

  auto jpeg_album = *jpeg_album_iter;
  auto jpeg_photos = jpeg_album->get_photos();
  constexpr auto expected_jpeg_photos = 4;
  REQUIRE(jpeg_photos.size() == expected_jpeg_photos);

  auto jpeg_files = jpeg_album->get_files();
  constexpr auto expected_jpeg_files = 4;
  REQUIRE(jpeg_files.size() == expected_jpeg_files);
}

TEST_CASE("Update albums", "[album][albums]") {
  // Create a temporary directory
  const auto temp_dir = album_architect::util::AutoTempDirectory {};
  fs::create_directory(temp_dir.path() / "album_1");
  fs::create_directory(temp_dir.path() / "album_2");

  SECTION("Test album updating") {
    // Create album with current information
    auto album = album_architect::Album::load_album(temp_dir.path());
    const auto expected_albums = 2;
    REQUIRE(album->get_albums().size() == expected_albums);

    // .. create new album should keep the cache
    fs::create_directory(temp_dir.path() / "album_3");
    REQUIRE(album->get_albums().size() == expected_albums);

    // .. updating cache should discover new album
    album->update_album();
    REQUIRE(album->get_albums().size() == expected_albums + 1);
  }

  // Create photos
  SECTION("Test photo updating") {
    const auto& path = temp_dir.path();
    album_architect::util::create_test_image(path / "image1.png", 128, 128);
    album_architect::util::create_test_image(path / "image2.jpg", 128, 64);

    auto album = album_architect::Album::load_album(path);
    constexpr auto expected_images = 2;
    REQUIRE(album->get_photos().size() == expected_images);

    album_architect::util::create_test_image(path / "image3.bmp", 64, 128);
    REQUIRE(album->get_photos().size() == expected_images);

    album->update_album();
    REQUIRE(album->get_photos().size() == expected_images + 1);
  }
}

TEST_CASE("Serialization of metadata", "[album][serialization][metadata]") {
  auto metadata = album_architect::PhotoMetadata {
      "creation_time", "date_time", "description", {"key1", "value1"}};

  auto stream = std::stringstream {};
  {
    auto output = cereal::BinaryOutputArchive(stream);
    output(metadata);
  }

  auto loaded_metadata = album_architect::PhotoMetadata {};
  {
    auto input = cereal::BinaryInputArchive(stream);
    input(loaded_metadata);
  }

  REQUIRE(metadata == loaded_metadata);
}

TEST_CASE("Serialization of data", "[album][serialization]") {
  // Test Photo serialization
  const auto temp_dir = album_architect::util::AutoTempDirectory();
  const auto& path = temp_dir.path();
  album_architect::util::create_test_image(path / "image1.png", 128, 128);

  SECTION("Test Photo serialization") {
    auto average_hash = album_architect::Hash<cv::img_hash::AverageHash> {};
    auto average_hash_loaded =
        album_architect::Hash<cv::img_hash::AverageHash> {};
    auto stream = std::stringstream {};

    // .. try saving
    {
      auto image = Photo(path / "image1.png");
      average_hash = image.calculate_average_hash();

      auto output = cereal::BinaryOutputArchive(stream);
      output(image);
    }

    // .. try loading
    {
      auto image = Photo {};
      auto input = cereal::BinaryInputArchive(stream);
      input(image);
      average_hash_loaded = image.calculate_average_hash();
    }

    // .. compare hashes
    REQUIRE(album_architect::compare_hashes(average_hash, average_hash_loaded)
            == Catch::Approx(0.0));
  }

}