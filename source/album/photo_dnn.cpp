//
// Created by jorge on 11/10/2023.
//
// NOTE: File separated from photo.cpp as there are incompatibilities
// between definitions of IplImage for OpenImageIO and DLIB. Adding all
// dlib definitions here.

#include "photo.h"

#include <dlib/data_io.h>
#include <dlib/opencv.h>
#include <glog/logging.h>
#include <opencv2/imgproc.hpp>

#include "album/face_classifier.h"

namespace album_architect {

auto Photo::get_faces_dnn() -> std::vector<cv::Rect2f> {
  // Get detector
  auto detector = FaceClassifier::get_dlib_face_detector();

  // Convert image to dlib
  load_opencv();

  auto image_wrapper = dlib::cv_image<dlib::bgr_pixel>(m_image_cv);
  auto target_image = dlib::matrix<dlib::rgb_pixel> {};
  dlib::assign_image(target_image, image_wrapper);

  // Find the faces with the detector and convert to OpenCV
  auto found_faces = (*detector)(target_image);
  auto result = std::vector<cv::Rect2f> {};

  for (const auto& face : found_faces) {
    const auto rect = face.rect;
    result.emplace_back(static_cast<float>(rect.left()),
                        static_cast<float>(rect.top()),
                        static_cast<float>(rect.width()),
                        static_cast<float>(rect.height()));
  }

  return result;
}

}  // namespace album_architect