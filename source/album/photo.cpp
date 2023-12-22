//
// Created by jorge on 11/10/2023.
//

#include <sstream>
#include <utility>

#include "photo.h"

#include <OpenImageIO/imagebufalgo.h>
#include <album/face_classifier.h>
#include <album/text_classifier.h>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>
#include <opencv2/img_hash.hpp>
#include <opencv2/imgproc.hpp>

namespace album_architect {
/// Calculates a hash of the given type
/// \tparam T
/// \return
/* clang-format off */
/* Concepts are still not supported by clang-format */
template<class T>
  requires std::is_base_of_v<cv::img_hash::ImgHashBase, T>
/* clang-format on */
auto calculate_hash(cv::InputArray input) -> Hash<T> {
  // Create hash
  auto hasher = T::create();
  auto result = cv::Mat {};
  hasher->compute(input, result);

  return Hash<T> {result};
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
Photo::Photo(std::filesystem::path path,
             OIIO::ImageBuf&& image,
             PhotoMetadata&& metadata)
    : m_path(std::move(path))
    , m_image(std::move(image))
    , m_metadata(std::move(metadata)) {}

/// Helper function for calculating a specific hash
/// \tparam T
/// \param output
/// \param input
/// \return
/* clang-format off */
/* Concepts are still not supported by clang-format */
template<class T>
  requires std::is_base_of_v<cv::img_hash::ImgHashBase, T>
/* clang-format on */
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
auto Photo::load_metadata() -> bool {
  if (!is_ok()) {
    return false;
  }

  const auto& image_spec = m_image.spec();
  auto keywords = std::vector<std::string> {};
  auto raw_keywords = image_spec.extra_attribs.get_string("Keywords").str();
  boost::algorithm::split(keywords, raw_keywords, boost::is_any_of(";"));

  // ... remove empty
  auto keywords_remove_iter =
      std::remove_if(keywords.begin(),
                     keywords.end(),
                     [](const auto& str) { return str.empty(); });
  keywords.erase(keywords_remove_iter, keywords.end());

  m_metadata = PhotoMetadata {
      image_spec.extra_attribs.get_string("Exif:DateTimeOriginal"),
      image_spec.extra_attribs.get_string("DateTime"),
      image_spec.extra_attribs.get_string("ImageDescription"),
      std::move(keywords)};

  return true;
}
auto Photo::get_faces() -> std::vector<cv::Rect2f> {
  auto face_detector = FaceClassifier::get_opencv_face_detector();

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
auto Photo::calculate_average_hash() -> Hash<cv::img_hash::AverageHash> {
  if (!m_average_hash) {
    load_opencv();
    m_average_hash = calculate_hash<cv::img_hash::AverageHash>(m_image_cv);
  }
  return m_average_hash.value();
}
auto Photo::calculate_phash() -> Hash<cv::img_hash::PHash> {
  if (!m_phash) {
    load_opencv();
    m_phash = calculate_hash<cv::img_hash::PHash>(m_image_cv);
  }
  return m_phash.value();
}
auto Photo::calculate_color_moment_hash()
    -> Hash<cv::img_hash::ColorMomentHash> {
  if (!m_color_moment_hash) {
    load_opencv();
    m_color_moment_hash =
        calculate_hash<cv::img_hash::ColorMomentHash>(m_image_cv);
  }
  return m_color_moment_hash.value();
}
auto Photo::calculate_marr_hildreth_hash()
    -> Hash<cv::img_hash::MarrHildrethHash> {
  if (!m_marr_hildreth_hash) {
    load_opencv();
    m_marr_hildreth_hash =
        calculate_hash<cv::img_hash::MarrHildrethHash>(m_image_cv);
  }

  return m_marr_hildreth_hash.value();
}
auto Photo::get_text_ocr() -> std::vector<TextElement> {
  load_opencv();

  // Do photo conversion
  auto copy = cv::Mat {};
  if (m_image_cv.channels() == 4) {
    cv::cvtColor(m_image_cv, copy, cv::COLOR_BGRA2RGB);
  } else if (m_image_cv.channels() == 3) {
    cv::cvtColor(m_image_cv, copy, cv::COLOR_BGR2RGB);
  } else {
    copy = m_image_cv;
  }

  // Perform OCR
  auto classifier = TextClassifier::get_tesseract_classifier();
  if (!classifier) {
    return {};
  }

  const auto page_segmentation_mode = tesseract::PageSegMode::PSM_AUTO;
  classifier->SetPageSegMode(page_segmentation_mode);

  classifier->SetImage(copy.data,
                       copy.cols,
                       copy.rows,
                       copy.channels(),
                       static_cast<int>(copy.step));
  auto full_text = std::unique_ptr<char[]>(
      classifier->GetUTF8Text());  // NOLINT(*-avoid-c-arrays)
  auto result = std::vector<TextElement>();

  // ... iterate through each finding
  auto* lines = classifier->GetIterator();
  if (lines == nullptr) {
    return {};
  }

  const auto iterator_level = tesseract::RIL_TEXTLINE;
  auto keep_going = true;
  while (keep_going) {
    // Get bounding box, text and confidence
    int bottom {};
    int top {};
    int right {};
    int left {};
    if (lines->BoundingBox(iterator_level, &left, &top, &right, &bottom)) {
      auto rect = cv::Rect2f {static_cast<float>(left),
                              static_cast<float>(top),
                              static_cast<float>(right - left),
                              static_cast<float>(bottom - top)};

      auto raw_text = std::unique_ptr<char[]>(  // NOLINT(*-avoid-c-arrays)
          lines->GetUTF8Text(iterator_level));
      auto confidence = lines->Confidence(iterator_level);

      //      result.emplace_back(rect, std::string(raw_text.get()),
      //      confidence);
      auto element =
          TextElement {rect, std::string(raw_text.get()), confidence};
      result.push_back(std::move(element));
    }

    // Get next element
    keep_going = lines->Next(iterator_level);
  }

  return result;
}
auto Photo::get_metadata() const -> const PhotoMetadata& {
  return m_metadata;
}
auto Photo::is_ok() const -> bool {
  return m_image.initialized();
}
Photo::Photo(const std::filesystem::path& path)
    : m_path(path)
    , m_image(path.string()) {
  // Log the error in case of issues
  if (!m_image.initialized()) {
    LOG(ERROR) << "Couldn't load image " << path
               << ". Error: " << m_image.geterror();
  } else {
    load_metadata();
  }
}

auto operator<<(std::ostream& ostream, const PhotoMetadata& metadata)
    -> std::ostream& {
  ostream << "creation_time: " << metadata.creation_time
          << " date_time: " << metadata.date_time
          << " description: " << metadata.description
          << " keywords: " << boost::algorithm::join(metadata.keywords, ";");
  return ostream;
}
}  // namespace album_architect