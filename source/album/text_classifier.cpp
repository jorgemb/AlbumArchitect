//
// Created by jorge on 25/10/2023.
//
#include "text_classifier.h"

#include <config/config.h>

using namespace std::string_literals;

namespace album_architect {
auto TextClassifier::get_tesseract_classifier(const std::string& language)
    -> std::unique_ptr<tesseract::TessBaseAPI> {
  // Create and initialize API
  const auto data_path = Config::get_tesseract_ocr_model_directory().string();
  auto api_client = std::make_unique<tesseract::TessBaseAPI>();
  if (api_client->Init(data_path.c_str(), language.c_str()) != 0) {
    return {};
  }

  return api_client;
}
}  // namespace album_architect