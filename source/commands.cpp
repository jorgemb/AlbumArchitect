//
// Created by jorge on 10/30/24.
//

#include <algorithm>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <mutex>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include "commands.h"

#include <CLI/Error.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "album/photo.h"
#include "analysis/similarity_search.h"
#include "files/tree.h"

namespace album_architect::commands {

namespace rng = std::ranges;

namespace {
auto get_baseline(const CommonParameters& parameters)
    -> std::optional<files::FileTree> {
  auto file_tree = std::optional<files::FileTree> {};

  // Try to load the baseline
  // TODO: Validate that the root path points to the same value in the cache
  if (auto file_data = std::ifstream(parameters.cache_path)) {
    spdlog::info("Loading cache from {}", parameters.cache_path.string());
    file_tree = files::FileTree::from_stream(file_data);

    // Couldn't load cache
    if (!file_tree) {
      spdlog::error("Couldn't load cache from {}",
                    parameters.cache_path.string());
    } else if(file_tree && file_tree->get_root_element().get_path() != parameters.photos_base_path) {
      // Check if base path has changed
      spdlog::warn(
          "A different root path was provided. Recreating cache. Previous "
          "path: {}, new path: {}",
          file_tree->get_root_element().get_path().string(),
          parameters.photos_base_path.string());
      file_tree = {};
    }
  }

  // If no file tree was created then create a new one
  if (!file_tree) {
    spdlog::info("Creating new cache path at {}", parameters.cache_path.string());
    file_tree = files::FileTree::build(parameters.photos_base_path);
    if (!file_tree) {
      spdlog::error("Couldn't open photo base path: {}",
                    parameters.photos_base_path.string());
      return {};
    }
  }

  return file_tree;
}
}  // unnamed namespace

void perform_analysis(const CommonParameters& common,
                      const AnalysisParameters& analysis) {
  auto file_tree = get_baseline(common);
  if (!file_tree) {
    throw CLI::ValidationError("Error while creating file tree");
  }

  // Get the files list and create analysis object
  auto id_photo_map = std::map<analysis::PhotoId, files::Element> {};
  auto id_photo_map_mutex = std::mutex {};

  spdlog::info("Gathering information for similarity index");
  auto similarity_builder = analysis::SimilaritySearchBuilder {};
  auto files =
      std::vector<files::Element> {file_tree->begin(), file_tree->end()};
  std::for_each(
      std::execution::par_unseq,
      files.begin(),
      files.end(),
      [&id_photo_map, &similarity_builder, &id_photo_map_mutex](auto& element)
      {
        if (auto photo = album::Photo::load(element)) {
          auto photo_id = similarity_builder.add_photo(*photo);

          auto guard = std::lock_guard(id_photo_map_mutex);
          id_photo_map.emplace(photo_id, element);
        }
      });

  // Make an analysis object
  spdlog::debug("Building similarity index");
  auto similarity = similarity_builder.build_search();

  // Report object
  auto report = nlohmann::json {};

  // Duplicates
  if (analysis.analyze_duplicates) {
    spdlog::info("Performing duplicates analysis");
    auto duplicates = similarity.get_duplicated_photos();
    spdlog::info("Found {} duplicate photos.", duplicates.size());

    // Add to the report
    auto report_duplicates = nlohmann::json::array();
    for (auto const& current : duplicates) {
      auto group = nlohmann::json::array();
      rng::transform(current,
                     std::back_inserter(group),
                     [&id_photo_map](const auto& photo_id)
                     { return id_photo_map.at(photo_id).get_path().string(); });

      report_duplicates.emplace_back(std::move(group));
    }
    report["duplicates"] = report_duplicates;
  }

  spdlog::info("Performing similar photo analysis with {} photos.",
               analysis.similar_photos_to_check.size());
  auto report_similars = nlohmann::json::object();
  for (auto const& current_photo : analysis.similar_photos_to_check) {
    // Try loading the image
    auto image = album::Image::load(current_photo);
    if (!image) {
      spdlog::error("Couldn't load image from {}", current_photo.string());
      continue;
    }

    // Get the similarity hash
    auto current_similars = similarity.get_similars_of(*image);
    auto report_current = nlohmann::json::array();
    rng::transform(
        current_similars,
        std::back_inserter(report_current),
        [&id_photo_map](const auto& id_similarity_pair)
        {
          auto result = fmt::format(
              R"({{"path": "{}", "similarity" : {} }})",
              id_photo_map.at(id_similarity_pair.first).get_path().string(),
              id_similarity_pair.second);
          return nlohmann::json::parse(result);
        });
    report_similars[current_photo.string()] = std::move(report_current);
  }
  report["similars"] = report_similars;

  // Store the final baseline
  if (auto output_file = std::ofstream(common.cache_path)) {
    spdlog::info("Writing to cache file: {}", common.cache_path.string());
    file_tree->to_stream(output_file);
  }

  // Write the report
  if (!analysis.output_path) {
    spdlog::info("Writing report to stdout");
    fmt::println("{}", report.dump());
  } else {
    spdlog::info("Writing report to {}", analysis.output_path->string());
    auto output_file = std::ofstream(*analysis.output_path);
    output_file << report.dump();
  }
}
}  // namespace album_architect::commands