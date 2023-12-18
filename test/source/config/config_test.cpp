#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include <album/util.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <config/config.h>
#include <fmt/core.h>

using namespace std::string_literals;
namespace fs = std::filesystem;

using album_architect::Config;

TEST_CASE("Basic configuration load", "[config]") {
  const auto temp_dir = album_architect::util::AutoTempDirectory {};
  auto current_dir =
      std::make_unique<album_architect::util::AutoSetWorkingDirectory>(
          temp_dir.path());

  // Write in the standard directory
  const auto cv_classifier = "cv.onnx"s;
  const auto dlib_classifier = "dlib.dat"s;
  const auto tesseract_dir = "test_data/"s;
  {
    const auto config = fmt::format(R"(
paths:
  cv_face_classifier: {}
  dlib_face_classifier: {}
  tesseract_ocr_directory: {}
)",
                                    cv_classifier,
                                    dlib_classifier,
                                    tesseract_dir);
    auto output = std::ofstream("config.yaml");
    output << config;
  }

  SECTION("Generic values") {
    REQUIRE_FALSE(Config::get_value("not_existant"));
    REQUIRE_FALSE(Config::get_value("path.does.not.exist"));

    // Getting the same path twice should give the same result
    Config::clear();
    REQUIRE(Config::get_value("paths.cv_face_classifier").value()
            == cv_classifier);
    {
      INFO("Second check of path");
      REQUIRE(Config::get_value("paths.cv_face_classifier").value()
              == cv_classifier);
    }

    REQUIRE(Config::get_value("paths.dlib_face_classifier").value()
            == dlib_classifier);
  }

  SECTION("Specific values") {
    Config::load();

    REQUIRE(Config::get_cv_face_classifier_model() == fs::path(cv_classifier));
    REQUIRE(Config::get_dlib_face_classifier_model()
            == fs::path(dlib_classifier));
    REQUIRE(Config::get_tesseract_ocr_model_directory()
            == fs::path(tesseract_dir));
  }

  SECTION("Default values") {
    {
      auto touch_file = std::ofstream("empty.yaml");
    }
    Config::load("empty.yaml");

    // Check default values
    REQUIRE(Config::get_cv_face_classifier_model()
            == fs::path("data") / "face_detection_yunet_2023mar.onnx");
    REQUIRE(Config::get_dlib_face_classifier_model()
            == fs::path("data") / "mmod_human_face_detector.dat");
    REQUIRE(Config::get_tesseract_ocr_model_directory()
            == fs::path("3rdparty") / "tessdata_best");
  }

  // Return everything to default
  current_dir.reset();
  album_architect::Config::load("config.test.yaml");
}
