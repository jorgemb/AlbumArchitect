#include <iostream>
#include <random>
#include <string>

#include <album/photo.h>
#include <album/util.h>
#include <boost/algorithm/string/trim.hpp>
#include <glog/logging.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

void detect_elements();
using namespace std::string_literals;
namespace fs = std::filesystem;

auto select_random_photos(const fs::path& base_path, size_t amount)
    -> std::vector<fs::path> {
  // Get all paths
  auto paths = std::vector<fs::path> {};
  std::for_each(fs::recursive_directory_iterator(base_path),
                fs::recursive_directory_iterator(),
                [&paths](const auto& entry)
                {
                  if (entry.is_regular_file()) {
                    paths.push_back(entry.path());
                  }
                });
  LOG(INFO) << "Found " << paths.size() << " photos";

  // Select random paths
  auto random_generator = std::default_random_engine {1142};
  auto selector = std::uniform_int_distribution<size_t> {0, paths.size()};
  auto result = std::vector<fs::path> {};
  result.reserve(amount);

  for (int i = 0; i < amount; ++i) {
    auto idx = selector(random_generator);
    result.push_back(paths.at(idx));
  }

  return result;
}

auto main(int argc, char* argv[]) -> int {
  google::InitGoogleLogging(argv[0]);
  google::SetLogDestination(google::GLOG_INFO, "album_architect.log");
  FLAGS_timestamp_in_logfile_name = false;

  cv::redirectError(album_architect::util::handle_cv_log_messages);

  detect_elements();
  cv::waitKey();
  return 0;
}

void detect_elements() {
  auto base_path = fs::path("D:") / "OneDrive" / "Fotos";
  auto selected_paths = select_random_photos(base_path, 10);
  for (auto const& path : selected_paths) {
    //  auto file = "test/sample-images/Samples/HEIC/IMG_0378.HEIC"s;
    auto photo = album_architect::Photo(path);
    if (!photo.is_ok()) {
      LOG(ERROR) << "Couldn't load photo";
      continue;
    }
    LOG(INFO) << "Loaded photo: " << path;

    // Draw faces
    auto faces = photo.get_faces();
    auto original = photo.get_cv_mat();

    const auto color = cv::Scalar(255, 0, 0);
    for (const auto& face : faces) {
      cv::rectangle(original, face, color, 6);
    }

    // Draw found text
    const auto text_color = cv::Scalar(0, 255, 255);
    auto text = photo.get_text_ocr();
    for (auto& [rect, text, confidence] : text) {
      // Check if there is text in the OCR
      boost::trim(text);
      if (text.empty()) {
        continue;
      }

      cv::rectangle(original, rect, text_color, 3);
      std::cout << "\tText: " << text << " -- Confidence: " << confidence
                << "\n";
    }

    // Save
    const auto scale = 800.0F / static_cast<float>(original.size().width);
    cv::resize(original, original, cv::Size(), scale, scale);
    cv::imshow(path.string(), original);
  }
}
