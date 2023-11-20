//
// Created by jorge on 16/10/2023.
//

#ifndef ALBUMARCHITECT_FACECLASSIFIER_H
#define ALBUMARCHITECT_FACECLASSIFIER_H

#include <utility>

#include <opencv2/objdetect.hpp>

namespace album_architect {

/// Manages getting each of the available classifiers
class FaceClassifier {
public:
  /// Tries to load the YN face detector
  static auto get_opencv_face_detector() -> std::shared_ptr<cv::FaceDetectorYN>;
};

}  // namespace album_architect

#endif  // ALBUMARCHITECT_FACECLASSIFIER_H
