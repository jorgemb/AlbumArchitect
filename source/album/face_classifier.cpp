//
// Created by jorge on 16/10/2023.
//

#include "face_classifier.h"

#include <config/config.h>

namespace album_architect {

auto FaceClassifier::get_opencv_face_detector()
    -> std::shared_ptr<cv::FaceDetectorYN> {
  constexpr int default_size = 320;
  constexpr float default_score_threshold = 0.8F;

  return cv::FaceDetectorYN::create(
      Config::get_cv_face_classifier_model().string(),
      "",
      cv::Size(default_size, default_size),
      default_score_threshold);
}
auto FaceClassifier::get_dlib_face_detector()
    -> std::shared_ptr<DlibFacenetType> {
  auto detector = std::make_shared<DlibFacenetType>();
  dlib::deserialize(Config::get_dlib_face_classifier_model().string())
      >> *detector;

  return detector;
}
}  // namespace album_architect