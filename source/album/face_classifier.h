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
template<int32_t NumFilters, typename SUBNET>
using con5d = dlib::con<NumFilters, 5, 5, 2, 2, SUBNET>;
template<int32_t NumFilters, typename SUBNET>
using con5 = dlib::con<NumFilters, 5, 5, 1, 1, SUBNET>;

template<typename SUBNET>
using downsampler = dlib::relu<dlib::affine<
    con5d<32,
          dlib::relu<dlib::affine<
              con5d<32, dlib::relu<dlib::affine<con5d<16, SUBNET>>>>>>>>>;
template<typename SUBNET>
using rcon5 = dlib::relu<dlib::affine<con5<45, SUBNET>>>;

using dlib_facenet_type = dlib::loss_mmod<
    dlib::con<1,
              9,
              9,
              1,
              1,
              rcon5<rcon5<rcon5<downsampler<
                  dlib::input_rgb_image_pyramid<dlib::pyramid_down<6>>>>>>>>;

/// Manages getting each of the available classifiers
class FaceClassifier {
public:
  /// Tries to load the YN face detector
  static auto get_opencv_face_detector() -> std::shared_ptr<cv::FaceDetectorYN>;

  /// Tries to load the MMOD dlib face detector
  static auto get_dlib_face_detector() -> std::shared_ptr <dlib_facenet_type>;
};

}  // namespace album_architect

#endif  // ALBUMARCHITECT_FACECLASSIFIER_H
