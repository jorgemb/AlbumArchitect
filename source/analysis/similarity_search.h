//
// Created by jorge on 09/10/24.
//

#ifndef ALBUMARCHITECT_SIMILARITY_SEARCH_H
#define ALBUMARCHITECT_SIMILARITY_SEARCH_H
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "album/photo.h"

namespace album_architect::analysis {
// Forward declarations
class SimilarityIndex;

// Represents the unique ID of a given photo
using PhotoId = std::size_t;

/// Class to perform a similarity search on the given photos
class SimilaritySearch {
public:
  /// Default constructor with a
  /// @param similarity_index
  explicit SimilaritySearch(std::unique_ptr<SimilarityIndex> similarity_index);

  /// Default destructor
  ~SimilaritySearch();

  // Delete copy, default move
  SimilaritySearch(const SimilaritySearch& other) = delete;
  SimilaritySearch(SimilaritySearch&& other) noexcept = default;
  auto operator=(const SimilaritySearch& other) -> SimilaritySearch& = delete;
  auto operator=(SimilaritySearch&& other) noexcept
      -> SimilaritySearch& = default;

  /// Returns all duplicated photo IDs
  /// @return
  auto get_duplicated_photos() const -> std::vector<std::vector<PhotoId>>;

  /// Returns all the duplicates of a given photo (including itself)
  /// @param photo
  /// @return
  auto get_duplicates_of(album::Photo& photo) const -> std::vector<PhotoId>;

  /// Returns a list of all the photos that are similar to the provided one
  /// @param photo
  /// @param similarity_threshold
  /// @param max_photos
  /// @return
  // NOLINTBEGIN(*-magic-numbers)
  auto get_similars_of(album::Photo& photo,
                       float similarity_threshold = 0.8F,
                       std::size_t max_photos = 100U) const
      -> std::vector<std::pair<PhotoId, std::uint8_t>>;
  // NOLINTEND(*-magic-numbers)

  /// Returns a list of all the photos that are similar to the provided one
  /// @param image
  /// @param similarity_threshold
  /// @param max_photos
  /// @return
  // NOLINTBEGIN(*-magic-numbers)
  auto get_similars_of(album::Image& image,
                       float similarity_threshold = 0.8F,
                       std::size_t max_photos = 100U) const
      -> std::vector<std::pair<PhotoId, std::uint8_t>>;
  // NOLINTEND(*-magic-numbers)

private:
  std::unique_ptr<SimilarityIndex> m_similarity_index;

  /// Contains internal helper functions
  struct HelperFunctions;
};

/// Helper class for creating the index that will be used later for
/// similarity search.
class SimilaritySearchBuilder {
public:
  /// Default constructor
  SimilaritySearchBuilder();

  /// Default destructor
  ~SimilaritySearchBuilder();

  /* Copy and move operations */
  SimilaritySearchBuilder(const SimilaritySearchBuilder& other) = delete;
  SimilaritySearchBuilder(SimilaritySearchBuilder&& other) noexcept = delete;
  auto operator=(const SimilaritySearchBuilder& other)
      -> SimilaritySearchBuilder& = delete;
  auto operator=(SimilaritySearchBuilder&& other) noexcept
      -> SimilaritySearchBuilder& = delete;

  /// Adds a photo to the index builder and returns a unique ID. This
  /// function should be thread safe.
  /// @param photo
  /// @return
  auto add_photo(album::Photo& photo) -> PhotoId;

  auto build_search() -> SimilaritySearch;

private:
  std::unique_ptr<class SimilarityIndex> m_similarity_index;
  std::size_t m_current_id = 0;

  std::mutex m_add_mutex;
};

}  // namespace album_architect::analysis

#endif  // ALBUMARCHITECT_SIMILARITY_SEARCH_H
