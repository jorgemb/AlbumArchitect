//
// Created by jorge on 10/30/24.
//

#include <algorithm>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "commands.h"

#include <CLI/Error.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "album/photo.h"
#include "analysis/similarity_search.h"
#include "files/tree.h"

namespace album_architect::commands {

namespace {
auto get_baseline(const CommonParameters& parameters)
    -> std::optional<files::FileTree> {
  auto file_tree = std::optional<files::FileTree> {};

  // Try to load the baseline
  // TODO: Validate that the root path points to the same value in the cache
  if (auto file_data = std::ifstream(parameters.cache_path)) {
    spdlog::info("Loading cache from {}", parameters.cache_path.string());
    file_tree = files::FileTree::from_stream(file_data);

    // Couldn't load cache, and baseline is not being updated. This means
    // we cannot continue
    if (!file_tree && !parameters.update_baseline) {
      spdlog::error("Couldn't load cache from {}",
                    parameters.cache_path.string());
      return {};
    }
  }

  // If no file tree was created then create a new one
  if (!file_tree) {
    spdlog::info("No cache file was loaded. Creating new one.");
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

  // Get the files list
  auto files = std::vector<files::Element> {};
  std::copy(file_tree->begin(), file_tree->end(), std::back_inserter(files));
  auto photos = std::vector<std::optional<album::Photo>> {};
  std::transform(files.begin(),
                 files.end(),
                 std::back_inserter(photos),
                 [](const auto& file) { return album::Photo::load(file); });

  // Remove invalid
  auto remove_iter = std::remove_if(
      photos.begin(), photos.end(), [](const auto& photo) { return !photo; });
  photos.erase(remove_iter, photos.end());

  // Make an analysis object
  auto similarity_builder = analysis::SimilaritySearchBuilder {};
  std::for_each(std::execution::par,
                photos.begin(),
                photos.end(),
                [&similarity_builder](auto& photo)
                {
                  similarity_builder.add_photo(*photo);

                  // Release the photo to free the memory
                  photo.reset();
                });
  auto similarity = similarity_builder.build_search();

  // Duplicates
  if (analysis.analyze_duplicates) {
    spdlog::info("Performing duplicates analysis");
  }

  if (!analysis.similar_photos_to_check.empty()) {
    spdlog::info("Performing similar photo analysis with {} photos.",
                 analysis.similar_photos_to_check.size());
  }

  // Store the final baseline
  if (common.update_baseline) {
    if (auto output_file = std::ofstream(common.cache_path)) {
      spdlog::info("Writing to cache file: {}", common.cache_path.string());
      file_tree->to_stream(output_file);
    }
  }
}
}  // namespace album_architect::commands