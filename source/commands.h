//
// Created by jorge on 10/30/24.
//

#ifndef COMMANDS_H
#define COMMANDS_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace album_architect::commands {

/// Common parameters expected for all commands
struct CommonParameters {
  std::filesystem::path photos_base_path;
  std::filesystem::path cache_path;
  bool update_baseline = false;
};

/// Parameters for performing analysis
struct AnalysisParameters {
  // Duplicates
  bool analyze_duplicates = false;
  // std::filesystem::path duplicates_start_path;  // Initial path to review

  // Similarities
  std::vector<std::filesystem::path> similar_photos_to_check;

  // Output
  std::optional<std::filesystem::path> output_path;
};

/// Analyzes the photos according to the parameters given
/// @param common
/// @param analysis
void perform_analysis(const CommonParameters& common,
                      const AnalysisParameters& analysis);

}  // namespace album_architect::commands

#endif  // COMMANDS_H
