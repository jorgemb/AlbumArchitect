#include <iostream>
#include <random>
#include <string>

#include <album/album.h>
#include <album/photo.h>
#include <album/util.h>
#include <config/config.h>
#include <fmt/core.h>
#include <glog/logging.h>

using namespace std::string_literals;
using namespace album_architect;
namespace fs = std::filesystem;

// auto select_random_photos(const fs::path& base_path, size_t amount
auto main(int argc, char* argv[]) -> int {
  // Init logging
  const auto log_destination = fs::current_path() / "album_architect.log";
  google::InitGoogleLogging(argv[0]);
  google::SetLogDestination(google::GLOG_INFO, log_destination.c_str());
  FLAGS_timestamp_in_logfile_name = false;

  // Init configuration
  Config::load("config/config.yaml");

  // Create the album
  auto album_list = Config::get_root_album_list();
  if (album_list.empty()) {
    LOG(ERROR) << "Root album list is empty";
    return -1;
  }

  auto root_album = Album::load_album(album_list.front());
  for (auto& photo : root_album->get_photos()) {
    const auto photo_path = photo->get_path();
    const auto photo_hash = photo->calculate_average_hash();
    fmt::print("Hash of ({}): {} -- {}\n",
               photo.use_count(),
               photo_path.filename().string(),
               album_architect::from_cv_to_hex(photo_hash.hash));

    photo.reset();
  }

  return 0;
}
