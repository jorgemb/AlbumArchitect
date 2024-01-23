//
// Created by jorge on 11/10/2023.
//

#include <filesystem>
#include <sstream>
#include <utility>

#include "photo.h"

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <album/face_classifier.h>
#include <album/text_classifier.h>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>
#include <opencv2/img_hash.hpp>
#include <opencv2/imgproc.hpp>
#include <range/v3/algorithm/remove_if.hpp>
#include <range/v3/range.hpp>

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

auto operator<<(std::ostream& ostream, const PhotoMetadata& obj)
    -> std::ostream& {
  return ostream << "creation_time: " << obj.creation_time
                 << " date_time: " << obj.date_time
                 << " description: " << obj.description
                 << " keywords: " << boost::algorithm::join(obj.keywords, ";")
                 << " width: " << obj.width << " height: " << obj.height;
}
auto Photo::get_path() const -> const std::filesystem::path& {
  return m_path;
}
auto Photo::get_width() const -> int64_t {
  return m_metadata.width;
}
auto Photo::get_height() const -> int64_t {
  return m_metadata.height;
}
Photo::Photo(std::filesystem::path path, PhotoMetadata&& metadata)
    : m_path(std::move(path))
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

auto Photo::get_cv_mat(const std::unique_ptr<OpenImageIO_v2_4::ImageBuf>& image)
    -> cv::Mat {
  if (!image) {
    return {};
  }

  auto image_cv = cv::Mat {};
  image->read(0, 0, /*force=*/true);
  const auto is_ok = OIIO::ImageBufAlgo::to_OpenCV(image_cv, *image);

  LOG_IF(ERROR, !is_ok)
      << "Couldn't create OpenCV image from loaded image. Error: "
      << OIIO::geterror();

  return image_cv;
}
auto Photo::load_metadata() -> bool {
  const auto image = load_image();
  if (!image) {
    return false;
  }

  const auto& image_spec = image->spec();
  auto keywords = std::vector<std::string> {};
  auto raw_keywords = image_spec.extra_attribs.get_string("Keywords").str();
  boost::algorithm::split(keywords, raw_keywords, boost::is_any_of(";"));

  // ... remove empty
  const auto keywords_remove_iter =
      ranges::remove_if(keywords, [](const auto& str) { return str.empty(); });
  keywords.erase(keywords_remove_iter, keywords.end());

  m_metadata = PhotoMetadata {
      image_spec.extra_attribs.get_string("Exif:DateTimeOriginal"),
      image_spec.extra_attribs.get_string("DateTime"),
      image_spec.extra_attribs.get_string("ImageDescription"),
      std::move(keywords),
      image_spec.width,
      image_spec.height};

  return true;
}
auto Photo::load_image() -> std::unique_ptr<OIIO::ImageBuf> {
  auto image = std::make_unique<OIIO::ImageBuf>(m_path.string());

  // Log the error in case of issues
  if (!image->initialized()) {
    LOG(ERROR) << "Couldn't load image " << m_path
               << ". Error: " << image->geterror();
    m_is_last_operation_ok = false;
    return {};
  }

  // Return the image
  m_is_last_operation_ok = true;
  return image;
}
auto Photo::get_faces() -> std::vector<cv::Rect2f> {
  const auto face_detector = FaceClassifier::get_opencv_face_detector();

  // TODO: Get configuration details from Config

  // Check if scaling is required
  const auto image = load_image();
  const auto image_cv = get_cv_mat(image);
  constexpr auto max_width = 800.0F;
  const auto scale =
      std::min(max_width / static_cast<float>(image_cv.size().width), 1.0F);

  auto target = cv::Mat {};
  cv::resize(image_cv, target, cv::Size(), scale, scale);

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
    if (const auto image = load_image()) {
      m_average_hash =
          calculate_hash<cv::img_hash::AverageHash>(get_cv_mat(image));
    } else {
      return {};
    }
  }
  return m_average_hash.value();
}
auto Photo::calculate_phash() -> Hash<cv::img_hash::PHash> {
  if (!m_phash) {
    if (const auto image = load_image()) {
      m_phash = calculate_hash<cv::img_hash::PHash>(get_cv_mat(image));
    } else {
      return {};
    }
  }
  return m_phash.value();
}
auto Photo::calculate_color_moment_hash()
    -> Hash<cv::img_hash::ColorMomentHash> {
  if (!m_color_moment_hash) {
    if (const auto image = load_image()) {
      m_color_moment_hash =
          calculate_hash<cv::img_hash::ColorMomentHash>(get_cv_mat(image));
    } else {
      return {};
    }
  }
  return m_color_moment_hash.value();
}
auto Photo::calculate_marr_hildreth_hash()
    -> Hash<cv::img_hash::MarrHildrethHash> {
  if (!m_marr_hildreth_hash) {
    if (const auto image = load_image()) {
      m_marr_hildreth_hash =
          calculate_hash<cv::img_hash::MarrHildrethHash>(get_cv_mat(image));
    } else {
      return {};
    }
  }
  return m_marr_hildreth_hash.value();
}
auto Photo::get_text_ocr() -> std::vector<TextElement> {
  // Do photo conversion
  const auto image = load_image();
  const auto image_cv = get_cv_mat(image);
  auto copy = cv::Mat {};
  if (image_cv.channels() == 4) {
    cv::cvtColor(image_cv, copy, cv::COLOR_BGRA2RGB);
  } else if (image_cv.channels() == 3) {
    cv::cvtColor(image_cv, copy, cv::COLOR_BGR2RGB);
  } else {
    copy = image_cv;
  }

  // Perform OCR
  const auto classifier = TextClassifier::get_tesseract_classifier();
  if (!classifier) {
    return {};
  }

  constexpr auto page_segmentation_mode = tesseract::PageSegMode::PSM_AUTO;
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

  constexpr auto iterator_level = tesseract::RIL_TEXTLINE;
  auto keep_going = true;
  while (keep_going) {
    // Get bounding box, text and confidence
    int bottom {};
    int top {};
    int right {};
    int left {};
    if (lines->BoundingBox(iterator_level, &left, &top, &right, &bottom)) {
      const auto rect = cv::Rect2f {static_cast<float>(left),
                                    static_cast<float>(top),
                                    static_cast<float>(right - left),
                                    static_cast<float>(bottom - top)};

      auto raw_text = std::unique_ptr<char[]>(  // NOLINT(*-avoid-c-arrays)
          lines->GetUTF8Text(iterator_level));
      const auto confidence = lines->Confidence(iterator_level);

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
  return m_is_last_operation_ok;
}
Photo::Photo(std::filesystem::path path)
    : m_path(std::move(path)) {
  load_metadata();
}

auto from_cv_to_hex(const cv::Mat& mat) -> std::string {
  std::ostringstream hex_string_stream;

  for (int row = 0; row < mat.rows; ++row) {
    const uchar* ptr = mat.ptr(row);

    for (int col = 0; col < mat.cols; ++col) {
      hex_string_stream << std::hex << std::setw(2) << std::setfill('0')
                        << static_cast<int>(*ptr++) << " ";
    }
  }

  return hex_string_stream.str();
}
}  // namespace album_architect