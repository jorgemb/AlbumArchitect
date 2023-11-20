//
// Created by jorge on 30/10/2023.
//

#include <random>

#include "util.h"

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <glog/logging.h>
#include <opencv2/core/utils/logger.defines.hpp>

namespace album_architect::util {

namespace fs = std::filesystem;

auto handle_cv_log_messages(int status,
                            const char* /*unused*/,
                            const char* err_msg,
                            const char* file_name,
                            int line,
                            void* /*unused*/) -> int {
  // Convert severity
  auto severity = google::GLOG_0;
  switch (status) {
    case CV_LOG_LEVEL_DEBUG:
    case CV_LOG_LEVEL_INFO:
      severity = google::GLOG_INFO;
      break;
    case CV_LOG_LEVEL_WARN:
      severity = google::GLOG_WARNING;
      break;
    case CV_LOG_LEVEL_ERROR:
      severity = google::GLOG_ERROR;
      break;
    case CV_LOG_LEVEL_FATAL:
      severity = google::GLOG_FATAL;
      break;
    default:
      severity = google::GLOG_INFO;
      break;
  }

  // Create message
  google::LogMessage(file_name, line, severity).stream() << err_msg;

  return 0;
}
auto AutoTempDirectory::generate_random_name(size_t size) -> std::string {
  const std::string chars(
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "1234567890"
      "_");

  // Create random device and generator
  auto generator = std::default_random_engine {std::random_device {}()};
  auto distribution =
      std::uniform_int_distribution<size_t>(0, chars.size() - 1);
  auto result = std::string(size, '_');
  for (auto& elem : result) {
    elem = chars[distribution(generator)];
  }

  return result;
}
AutoTempDirectory::AutoTempDirectory() {
  const auto temp_dir = fs::temp_directory_path();

  bool created = false;
  while (!created) {
    const auto name = generate_random_name();
    m_path = temp_dir / name;

    // Check that the directory doesn't exist and that it can be created
    if (fs::exists(m_path)) {
      continue;
    }

    if (!fs::create_directory(m_path)) {
      continue;
    }

    created = true;
  }
}
AutoTempDirectory::~AutoTempDirectory() {
  // Try removing all subdirectories of the path
  fs::remove_all(m_path);
}
auto create_test_image(const std::filesystem::path& path, int width, int height)
    -> bool {
  // Fill buffer with data
  OIIO::ImageSpec spec(width, height, 3, OIIO::TypeDesc::FLOAT);
  OIIO::ImageBuf buffer(spec);
  constexpr auto dark = std::array<float, 3> {0.1, 0.1, 0.1};
  constexpr auto light = std::array<float, 3> {0.8, 0.8, 0.8};
  constexpr auto square_size = 64;
  OIIO::ImageBufAlgo::checker(buffer, square_size, square_size, 1, dark, light);

  return buffer.write(path.string());
}
}  // namespace album_architect::util
