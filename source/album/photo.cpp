//
// Created by jorge on 11/10/2023.
//

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

#include "photo.h"

#include <glog/logging.h>
#include <opencv2/img_hash.hpp>
#include <opencv2/imgproc.hpp>
#include <openimageio/imagebuf.h>
#include <openimageio/imagebufalgo.h>

#include "album/face_classifier.h"

namespace album_architect {

auto Photo::load(const std::filesystem::path& path) -> std::unique_ptr<Photo> {
  // Check if path exists
  if (!exists(path)) {
    LOG(ERROR) << "Path doesn't exist: " << path;
    return {};
  }

  // Load the photo
  auto image = OIIO::ImageBuf(path.string());
  if (!image.initialized()) {
    // Error while reading image
    LOG(ERROR) << "Couldn't get image information for: " << path
               << ". Error: " << image.geterror();
    return {};
  }

  return std::unique_ptr<Photo>(new Photo(path, std::move(image)));
}

auto Photo::get_path() const -> const std::filesystem::path& {
  return m_path;
}
auto Photo::get_width() const -> int64_t {
  return m_image.spec().width;
}
auto Photo::get_height() const -> int64_t {
  return m_image.spec().height;
}
Photo::Photo(std::filesystem::path path, OIIO::ImageBuf image)
    : m_path(std::move(path))
    , m_image(std::move(image)) {}

/// Helper function for calculating a specific hash
/// \tparam T
/// \param output
/// \param input
/// \return
template<class T>
  requires std::is_base_of_v<cv::img_hash::ImgHashBase, T>
auto calculate_hash(cv::InputArray input, cv::OutputArray output) -> void {
  auto hasher = T::create();
  hasher->compute(input, output);
}

void Photo::load_opencv() {
  if (m_image_cv.empty()) {
    m_image.read(0, 0, /*force=*/true);
    auto is_ok = OIIO::ImageBufAlgo::to_OpenCV(m_image_cv, m_image);

    LOG_IF(ERROR, !is_ok)
        << "Couldn't create OpenCV image from loaded image. Error: "
        << OIIO::geterror();
  }
}
auto Photo::get_cv_mat() -> const cv::Mat& {
  load_opencv();
  return m_image_cv;
}
auto Photo::get_faces() -> std::vector<cv::Rect2f> {
  auto face_detector = FaceClassifier::get_face_detector();

  // Convert to gray and get faces
  load_opencv();

  // TODO: Get configuration details from Config

  // Check if scaling is required
  const auto max_width = 800.0F;
  const auto scale =
      std::min(max_width / static_cast<float>(m_image_cv.size().width), 1.0F);

  auto target = cv::Mat {};
  cv::resize(m_image_cv, target, cv::Size(), scale, scale);

  // Detect faces
  auto detected_faces = cv::Mat {};
  face_detector->setInputSize(target.size());
  face_detector->detect(target, detected_faces);

  // Extract faces as rects
  auto faces = std::vector<cv::Rect2f> {};
  for (auto i = 0; i < detected_faces.rows; ++i) {
    faces.emplace_back(detected_faces.at<float>(i, 0) / scale,
                       detected_faces.at<float>(i, 1) / scale,
                       detected_faces.at<float>(i, 2) / scale,
                       detected_faces.at<float>(i, 3) / scale);
  }

  return faces;
}

}  // namespace album_architect