//
// Created by jorge on 24/11/2023.
//

#include "config.h"

#include <boost/algorithm/string.hpp>
#include <glog/logging.h>
#include <yaml-cpp/yaml.h>

namespace album_architect {

std::unique_ptr<YAML::Node> Config::config_data = {};
using namespace std::string_literals;

auto Config::get_value(std::string_view key) -> std::optional<std::string> {
  // Do load if pointer does not exist
  if (!config_data) {
    load();
  }

  auto result = std::vector<std::string> {};
  boost::split(result, key, boost::is_any_of("."));

  // Create a stack for temporary values
  auto stack = std::vector {*config_data};
  stack.reserve(result.size());

  for (auto current_part = result.begin(); current_part != result.end();
       ++current_part)
  {
    if (auto& current_node = stack.back(); current_node[*current_part]) {
      stack.push_back(current_node[*current_part]);
    } else {
      return {};
    }
  }

  return stack.back().as<std::string>();
}
auto Config::load(const std::filesystem::path& path) -> bool {
  // Creates an empty node if not existent
  if (!config_data) {
    config_data = std::make_unique<YAML::Node>();
  }

  // Tries loading the configuration
  try {
    auto node = YAML::LoadFile(path.string());
    config_data = std::make_unique<YAML::Node>(node);
    return true;
  } catch (const YAML::ParserException&) {
    LOG(ERROR) << "Couldn't parse file at " << path;
    return false;
  } catch (const YAML::BadFile&) {
    LOG(ERROR) << "Couldn't load file at " << path;
    return false;
  }
}
auto Config::get_cv_face_classifier_model() -> std::filesystem::path {
  return get_value("paths.cv_face_classifier")
      .value_or("data/face_detection_yunet_2023mar.onnx"s);
}
auto Config::get_dlib_face_classifier_model() -> std::filesystem::path {
  return get_value("paths.dlib_face_classifier")
      .value_or("data/mmod_human_face_detector.dat"s);
}
auto Config::get_tesseract_ocr_model_directory() -> std::filesystem::path {
  return get_value("paths.tesseract_ocr_directory")
      .value_or("3rdparty/tessdata_best"s);
}
}  // namespace album_architect