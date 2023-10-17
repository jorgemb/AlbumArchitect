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
  auto file = "test/sample-images/Samples/HEIC/IMG_0378.HEIC"s;
  auto photo = album_architect::Photo::load(file);

  auto faces = photo->get_faces();
  std::cout << "Found: " << faces.size() << " faces\n";

  auto original = photo->get_cv_mat();

  // Draw faces
  const auto color = cv::Scalar(255, 0, 0);
  for(const auto &face: faces){
    cv::rectangle(original, face, color, 2);
  }

  // Save
  cv::imwrite("output.png", original);

  cv::waitKey();
  return 0;
}
