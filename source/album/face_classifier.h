//
// Created by jorge on 16/10/2023.
//

#ifndef ALBUMARCHITECT_FACECLASSIFIER_H
#define ALBUMARCHITECT_FACECLASSIFIER_H

#include <utility>
#include <opencv2/face.hpp>

namespace album_architect {

enum class HaarCascade{
  frontal_face,
  full_body,
  profile_face,
};

/// Manages getting each of the available classifiers
class FaceClassifier {
public:
  /// Returns a loaded classifier
  /// \param type
  /// \return
  static auto get_cascade_classifier(HaarCascade type) -> std::shared_ptr<cv::CascadeClassifier>;
};

}  // namespace album_architect

#endif  // ALBUMARCHITECT_FACECLASSIFIER_H
