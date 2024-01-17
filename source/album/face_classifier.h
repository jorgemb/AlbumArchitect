//
// Created by jorge on 16/10/2023.
//

#ifndef ALBUMARCHITECT_FACECLASSIFIER_H
#define ALBUMARCHITECT_FACECLASSIFIER_H

#include <utility>

#include <dlib/dnn.h>
#include <opencv2/objdetect.hpp>

namespace album_architect {

// DLIB types for face recognition
// NOLINTBEGIN
template<int32_t NumFilters, typename SUBNET>
using Con5d = dlib::con<NumFilters, 5, 5, 2, 2, SUBNET>;
template<int32_t NumFilters, typename SUBNET>
using Con5 = dlib::con<NumFilters, 5, 5, 1, 1, SUBNET>;

template<typename SUBNET>
using Downsampler = dlib::relu<dlib::affine<
    Con5d<32,
          dlib::relu<dlib::affine<
              Con5d<32, dlib::relu<dlib::affine<Con5d<16, SUBNET>>>>>>>>>;
template<typename SUBNET>
using Rcon5 = dlib::relu<dlib::affine<Con5<45, SUBNET>>>;

using DlibFacenetType = dlib::loss_mmod<
    dlib::con<1,
              9,
              9,
              1,
              1,
              Rcon5<Rcon5<Rcon5<Downsampler<
                  dlib::input_rgb_image_pyramid<dlib::pyramid_down<6>>>>>>>>;
// NOLINTEND

/// Manages getting each of the available classifiers
class FaceClassifier {
public:
  /// Tries to load the YN face detector
  static auto get_opencv_face_detector() -> std::shared_ptr<cv::FaceDetectorYN>;

  /// Tries to load the MMOD dlib face detector
  static auto get_dlib_face_detector() -> std::shared_ptr <DlibFacenetType>;
};

}  // namespace album_architect

#endif  // ALBUMARCHITECT_FACECLASSIFIER_H
