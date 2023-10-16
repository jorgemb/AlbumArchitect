#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/face.hpp>
#include <album/photo.h>

#include "lib.hpp"

using namespace std::string_literals;

auto main() -> int {
  auto file = "img.png"s;
  auto photo = album_architect::Photo::load(file);
  auto mat = photo->get_cv_mat();
  cv::imshow("Original", mat);

  auto gray = cv::Mat{};
  cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
  cv::imshow("Gray", gray);

  auto cascades_name = "data/haarcascades/haarcascade_frontalface_default.xml"s;
  auto classifier = cv::CascadeClassifier(cascades_name);

  auto faces = std::vector<cv::Rect>{};
  classifier.detectMultiScale(gray, faces);

  std::cout << "Found: " << faces.size() << " faces\n";

  // Draw faces
  const auto color = cv::Scalar(255, 0, 0);
  for(const auto &face: faces){
    cv::rectangle(mat, face, color, 2);
  }
  cv::imshow("Faces", mat);

  cv::waitKey();
  return 0;
}
