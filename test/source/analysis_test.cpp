#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "album/photo.h"
#include "analysis/similarity_search.h"
#include "common.h"
#include "files/tree.h"

using namespace album_architect;  // NOLINT(*-build-using-namespace)
namespace rng = std::ranges;
namespace vws = std::ranges::views;

/// Loads all photos that are in a file tree
/// @param tree
/// @return
auto load_photos(files::FileTree& tree) -> std::vector<album::Photo> {
  auto loaded_elements = std::vector<files::Element> {};
  std::copy(tree.begin(), tree.end(), std::back_inserter(loaded_elements));

  auto photos = std::vector<album::Photo> {};
  {
    auto loaded_photos = std::vector<std::optional<album::Photo>> {};
    rng::transform(
        loaded_elements, std::back_inserter(loaded_photos), album::Photo::load);

    const auto last = std::remove_if(loaded_photos.begin(),
                                     loaded_photos.end(),
                                     [](const auto& photo) { return !photo; });

    std::transform(std::make_move_iterator(loaded_photos.begin()),
                   std::make_move_iterator(last),
                   std::back_inserter(photos),
                   [](auto&& photo) { return std::move(photo.value()); });
  }

  return photos;
}

TEST_CASE("Similarity test", "[SimilarityTest]") {
  // Basic initialization
  const auto images_dir = resources_dir / "images";
  auto file_tree = files::FileTree::build(images_dir);
  REQUIRE(file_tree);

  // Get all photos
  auto photos = load_photos(*file_tree);

  // Add photos to the similarity builder
  auto similarity_builder = analysis::SimilaritySearchBuilder {};

  auto photo_ids = std::vector<analysis::PhotoId> {};
  rng::transform(photos,
                 std::back_inserter(photo_ids),
                 [&similarity_builder](auto& photo)
                 { return similarity_builder.add_photo(photo); });
  auto unique_start = std::unique(photo_ids.begin(), photo_ids.end());
  photo_ids.erase(unique_start, photo_ids.end());
  REQUIRE(photos.size() == photo_ids.size());

  // Add the first photo a total of 3 times to have similar on search
  similarity_builder.add_photo(photos[0]);
  similarity_builder.add_photo(photos[0]);

  // Add the second photo a total of 2 times
  similarity_builder.add_photo(photos[1]);

  // Build the index and search for duplicates
  auto similarity_search = similarity_builder.build_search();

  {
    auto duplicate_photos = similarity_search.get_duplicated_photos();
    constexpr auto expected_duplicate_groups =
        3;  // Duplicates of first, second and (console.tiff + console.png)
    REQUIRE(duplicate_photos.size() == expected_duplicate_groups);

    auto duplicates_of_first = similarity_search.get_duplicates_of(photos[0]);
    REQUIRE(duplicates_of_first.size() == 3);

    auto duplicates_of_second = similarity_search.get_duplicates_of(photos[1]);
    REQUIRE(duplicates_of_second.size() == 2);

    auto other_element =
        file_tree->get_element(images_dir / "Home" / "IMG_5515.JPG");
    auto other_photo = album::Photo::load(other_element.value());
    auto duplicates_of_other =
        similarity_search.get_duplicates_of(*other_photo);
    REQUIRE(duplicates_of_other.size() == 1);
  }
}