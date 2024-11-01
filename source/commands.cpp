//
// Created by jorge on 10/30/24.
//

#include <fstream>
#include <optional>
#include <string>
#include <filesystem>

#include "commands.h"

#include <CLI/Error.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "files/tree.h"

namespace album_architect::commands {

namespace {
auto get_baseline(const CommonParameters& parameters)
    -> std::optional<files::FileTree> {
  if (!parameters.update_baseline) {
    spdlog::info("Not updating baseline");

    // Validate that the baseline exists
    if (!exists(parameters.cache_path)) {
      spdlog::error("Couldn't find cache file: {}",
                    parameters.cache_path.string());
      return {};
    }

    // Try to load the baseline
    auto file_data = std::ifstream(parameters.cache_path);
    auto file_tree = files::FileTree::from_stream(file_data);
    if (!file_tree) {
      spdlog::error("Couldn't open cache file: {}",
                    parameters.cache_path.string());
      return {};
    }

    return file_tree;
  }

  spdlog::info("Updating baseline for path: {}",
               parameters.photos_base_path.string());
  auto file_tree = files::FileTree::build(parameters.photos_base_path.string());
  if (!file_tree) {
    spdlog::error("Couldn't open photo base path: {}",
                  parameters.photos_base_path.string());
    return {};
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