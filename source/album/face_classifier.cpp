//
// Created by jorge on 16/10/2023.
//

#include <filesystem>
#include <config/config.h>

#include "face_classifier.h"

namespace fs = std::filesystem;

namespace album_architect {

auto FaceClassifier::get_opencv_face_detector()
    -> std::shared_ptr<cv::FaceDetectorYN> {
  return cv::FaceDetectorYN::create(
      Config::get_cv_face_classifier_model().string(), "", cv::Size(320, 320), 0.8F);
}
auto FaceClassifier::get_dlib_face_detector()
    -> std::shared_ptr<dlib_facenet_type> {
  auto detector = std::make_shared<dlib_facenet_type>();
  dlib::deserialize(Config::get_dlib_face_classifier_model().string()) >> *detector;

  return detector;
}
}  // namespace album_architect