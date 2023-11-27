//
// Created by jorge on 24/11/2023.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>
#include <memory>
#include <string>
#include <optional>

namespace YAML { // NOLINT(*-identifier-naming)
// Forward declaration
class Node;
}  // namespace YAML

namespace album_architect {

/// Class for managing configuration files
class Config final {
  static std::unique_ptr<YAML::Node> config_data;

public:
  /// \brief Returns a value from a key
  /// \param key
  /// \return
  static auto get_value(std::string_view key) -> std::optional<std::string>;

  /// \brief Loads the configuration information in the given path
  /// \param path
  /// \return
  static auto load(const std::filesystem::path& path = "config.yaml") -> bool;
  static auto get_cv_face_classifier_model() -> std::filesystem::path;
  static auto get_dlib_face_classifier_model() -> std::filesystem::path;
  static auto get_tesseract_ocr_model_directory() -> std::filesystem::path;
};

}  // namespace album_architect

#endif  // CONFIG_H
