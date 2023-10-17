//
// Created by jorge on 16/10/2023.
//

#include <filesystem>

#include "face_classifier.h"

namespace fs = std::filesystem;

namespace album_architect {
auto FaceClassifier::get_cascade_classifier(HaarCascade type)
    -> std::shared_ptr<cv::CascadeClassifier> {
  // TODO: Get base path from configuration
  auto classifier_path = fs::path("data/haarcascades");
  switch (type) {
    case HaarCascade::frontal_face:
      classifier_path = classifier_path / "haarcascade_frontalface_default.xml";
      break;
    case HaarCascade::full_body:
      classifier_path = classifier_path / "haarcascade_fullbody.xml";
      break;
    case HaarCascade::profile_face:
      classifier_path = classifier_path / "haarcascade_profileface.xml";
      break;
  }

  // Load classifier
  return std::make_shared<cv::CascadeClassifier>(classifier_path.string());
}
}  // namespace album_architect