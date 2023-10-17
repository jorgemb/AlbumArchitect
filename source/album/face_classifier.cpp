//
// Created by jorge on 16/10/2023.
//

#include <filesystem>

#include "face_classifier.h"

namespace fs = std::filesystem;

namespace album_architect {

auto FaceClassifier::get_face_detector()
    -> std::shared_ptr<cv::FaceDetectorYN> {
  const auto path = fs::path("data") / "face_detection_yunet_2023mar.onnx";

  // TODO: Get config input
  return cv::FaceDetectorYN::create(path.string(), "", cv::Size(320, 320),
                                    0.8F);
}
}  // namespace album_architect