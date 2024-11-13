// #include <algorithm>
// #include <deque>
// #include <execution>
// #include <fstream>
// #include <iostream>
// #include <iterator>
// #include <optional>
// #include <ranges>
// #include <utility>
// #include <vector>
//
// #include <CLI/CLI.hpp>
// #include <fmt/core.h>
// #include <spdlog/spdlog.h>
//
// #include "album/photo.h"
// #include "album/photo_metadata.h"
// #include "files/tree.h"
//
// namespace rng = std::ranges;
// namespace vws = std::ranges::views;
// using album_architect::album::ImageHashAlgorithm;
// using album_architect::album::PhotoMetadata;

#include <filesystem>
#include <string>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include "commands.h"

/// Validates that the given path points to a folder.
/// @param path
/// @return Returns empty string in case of success
auto is_directory_path(std::string path) -> std::string {
  if (!std::filesystem::is_directory(path)) {
    return fmt::format("Provided path: {} does not point to a directory.",
                       path);
  }
  return {};
}

auto main(int argc, char* argv[]) -> int {
  spdlog::set_level(spdlog::level::info);
  auto app = CLI::App {
      "AlbumArchitect is an application for help organizing photos, finding "
      "similar and duplicates."};

  // Get the common options
  auto common_parameters = album_architect::commands::CommonParameters {};
  app.add_option("--photos-path,-p",
                 common_parameters.photos_base_path,
                 "Path to the photos folder")
      ->required()
      ->check(is_directory_path);
  app.add_option("--cache-file,-c",
                 common_parameters.cache_path,
                 "Path to the cache file.")
      ->default_val(".albumarchitect.cache");
  app.add_flag_callback(
      "--verbose,-v",
      []() { spdlog::set_level(spdlog::level::debug); },
      "Increase verbosity. Can be sent several times.");

  // Analysis parameters
  auto analysis_parameters = album_architect::commands::AnalysisParameters {};
  auto* analyze_command =
      app.add_subcommand("analyze", "Perform an analysis on the photos")
          ->fallthrough()
          ->callback(
              [&common_parameters, &analysis_parameters]()
              {
                album_architect::commands::perform_analysis(
                    common_parameters, analysis_parameters);
              });
  analyze_command->add_flag("--analyze-duplicates,-d",
                            analysis_parameters.analyze_duplicates,
                            "Performs an analysis for full duplicates");
  // analyze_command
  //     ->add_option("--duplicate-start-path",
  //                  analysis_parameters.duplicates_start_path,
  //                  "Determine from which folder to start the duplicate
  //                  search.")
  //     ->default_val(common_parameters.photos_base_path)
  //     ->check(is_directory_path);
  analyze_command->add_option("--check-similars,-s",
                              analysis_parameters.similar_photos_to_check,
                              "Path to photos for which similar are being "
                              "searched for. Can be sent several times.");
  analyze_command->add_option(
      "--output,-o",
      analysis_parameters.output_path,
      "Path to the output report file. Defaults to stdout.");

  CLI11_PARSE(app, argc, argv);

  return 0;
}

// auto main(int argc, char* argv[]) -> int {
//   // Check usage
//   if (argc != 2) {
//     fmt::println("Usage: album_architect <FOLDER>");
//     return -1;
//   }
//
//   // Try to create path
//   auto path = fs::path(argv[1]);
//   if (!fs::exists(path) || !fs::is_directory(path)) {
//     spdlog::error("The given path {} is not valid.", path.string());
//     return -1;
//   }
//
//   // Create the file tree and output
//   const auto output_path = fs::path("filesystem.data");
//   auto file_tree = std::optional<album_architect::files::FileTree> {};
//   if (exists(output_path)) {
//     // Try loading from filesystem
//     spdlog::info("Trying to load from path {}", output_path.string());
//     auto file_data = std::ifstream(output_path);
//     file_tree = album_architect::files::FileTree::from_stream(file_data);
//     if (!file_tree) {
//       return -1;
//     }
//   } else {
//     // Calculate filetree
//     spdlog::info("Calculating file tree");
//     file_tree = album_architect::files::FileTree::build(path);
//     if (!file_tree) {
//       spdlog::error("Couldn't create a tree of path: {}", path.string());
//       return -1;
//     }
//   }
//
//   // Get all elements
//   auto is_file = [](const album_architect::files::Element& element)
//   { return element.get_type() == album_architect::files::PathType::file; };
//   auto file_elements = std::vector<album_architect::files::Element> {};
//   std::copy_if(file_tree->begin(),
//                file_tree->end(),
//                std::back_inserter(file_elements),
//                is_file);
//
//   // Load all hashes
//   spdlog::info("LOADING HASHES");
//   std::for_each(std::execution::par_unseq,
//                 file_elements.begin(),
//                 file_elements.end(),
//                 [](auto& element)
//                 {
//                   if (auto photo =
//                   album_architect::album::Photo::load(element))
//                   {
//                     photo->get_image_hash(ImageHashAlgorithm::average_hash);
//                     photo->get_image_hash(ImageHashAlgorithm::p_hash);
//                   }
//                 });
//
//   // .. serialize
//   spdlog::info("SERIALIZING");
//   {
//     if (auto serialize_output = std::ofstream(output_path)) {
//       file_tree->to_stream(serialize_output);
//     }
//   }
//
//   // ... write to graphviz
//   fmt::println("Writing to Graphviz");
//   if (auto output_file = std::ofstream("filesystem.dot")) {
//     file_tree->to_graphviz(output_file);
//   }
//
//   spdlog::info("DESTROYING");
//
//   return 0;
// }
