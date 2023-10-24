#include <iostream>
#include <random>
#include <string>

#include <album/photo.h>
#include <glog/logging.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/text.hpp>
#include <boost/range/combine.hpp>

void detect_faces();
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

//  detect_faces();
  auto image = album_architect::Photo::load("test/sample-images/Samples/BMP/error/33A3F6D37FE162E4.bmp");
//  auto image = album_architect::Photo::load("D:/OneDrive/Fotos/Freelance/2013-06-06 14.05.48.jpg");
  if(!image){
    return -1;
  }

  // Try to detect text
  auto detector = cv::text::OCRTesseract::create("3rdparty/tessdata_best", "eng");
  auto cv_image = image->get_cv_mat();
  auto copy = cv::Mat{};
  cv::cvtColor(cv_image, copy, cv::COLOR_BGRA2BGR);

  auto text_elements = std::vector<std::string>{};
  auto rect_elements = std::vector<cv::Rect>{};
  auto confidence_elements = std::vector<float>{};
  auto text = std::string{};
  detector->run(copy, text, &rect_elements, &text_elements, &confidence_elements);
  std::cout << "Detected text: " << text << "\n";

  // Draw rects
  for(auto [rect, confidence, text_extract]: boost::combine(rect_elements, confidence_elements, text_elements)){
    const auto conf_color = cv::Scalar(static_cast<int>(255.0F * confidence), 0, 0);
    cv::rectangle(copy, rect, conf_color);

    auto point = rect.tl();
    point.y -= 9;
    cv::putText(copy, text_extract, point, cv::FONT_HERSHEY_SIMPLEX, 0.3, conf_color);
  }

  cv::imshow("Image", copy);
  cv::waitKey();
  return 0;
}

void detect_faces() {
  auto base_path = fs::path("D:") / "OneDrive" / "Fotos";
  auto selected_paths = select_random_photos(base_path, 10);
  for (auto const& path : selected_paths) {
    //  auto file = "test/sample-images/Samples/HEIC/IMG_0378.HEIC"s;
    auto photo = album_architect::Photo::load(path);
    if (!photo) {
      LOG(ERROR) << "Couldn't load photo";
      continue;
    }
    LOG(INFO) << "Loaded photo: " << path;

    // Draw faces
    auto faces = photo->get_faces();
    auto original = photo->get_cv_mat();

    const auto color = cv::Scalar(255, 0, 0);
    for (const auto& face : faces) {
      cv::rectangle(original, face, color, 6);
    }

    // Save
    const auto scale = 800.0F / static_cast<float>(original.size().width);
    cv::resize(original, original, cv::Size(), scale, scale);
    cv::imshow(path.string(), original);
  }
}
