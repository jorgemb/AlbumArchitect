#include <iostream>
#include <random>
#include <string>

#include <album/photo.h>
#include <dlib/data_io.h>
#include <dlib/dnn.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/gui_widgets.h>
#include <glog/logging.h>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

void detect_faces();
using namespace std::string_literals;
namespace fs = std::filesystem;

using dlib::affine;
using dlib::con;
using dlib::input_rgb_image_pyramid;
using dlib::loss_mmod;
using dlib::pyramid_down;
using dlib::relu;

template<long NumFilters, typename SUBNET>
using con5d = con<NumFilters, 5, 5, 2, 2, SUBNET>;
template<long NumFilters, typename SUBNET>
using con5 = con<NumFilters, 5, 5, 1, 1, SUBNET>;

template<typename SUBNET>
using downsampler = relu<affine<
    con5d<32, relu<affine<con5d<32, relu<affine<con5d<16, SUBNET>>>>>>>>>;
template<typename SUBNET>
using rcon5 = relu<affine<con5<45, SUBNET>>>;

using net_type = loss_mmod<
    con<1,
        9,
        9,
        1,
        1,
        rcon5<rcon5<
            rcon5<downsampler<input_rgb_image_pyramid<pyramid_down<6>>>>>>>>;

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
  auto random_generator = std::default_random_engine {std::random_device {}()};
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
  auto path = fs::path("test/sample-images/Samples/HEIC/IMG_0378.HEIC");
  auto photo = album_architect::Photo::load(path);

  // Load network
  auto cv_mat = photo->get_cv_mat();
  auto net = net_type {};
  auto net_path = "data/dlib_face.dat"s;
  dlib::deserialize(net_path) >> net;

  auto image = dlib::cv_image<dlib::bgr_pixel>(cv_mat);
  auto converted_image = dlib::matrix<dlib::rgb_pixel> {};
  dlib::assign_image(converted_image, image);
  auto found_faces = net(converted_image);

  auto window = dlib::image_window{};
  window.clear_overlay();
  window.set_image(converted_image);

  const auto color = cv::Scalar(255, 0, 0);
  for (auto&& face : found_faces) {
//    auto rect = face.rect;
//    auto cv_rect =
//        cv::Rect(rect.left(), rect.top(), rect.width(), rect.height());
//    cv::rectangle(cv_mat, cv_rect, color, 4);
    window.add_overlay(face);
  }

  const auto scale = 800.0 / cv_mat.size().width;
//  cv::resize(cv_mat, cv_mat, cv::Size(), scale, scale);
//  cv::imshow("Image", cv_mat);


  std::cin.get();
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
    } else {
      LOG(INFO) << "Loaded photo: " << path;
    }

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
