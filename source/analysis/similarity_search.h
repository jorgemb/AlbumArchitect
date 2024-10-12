//
// Created by jorge on 09/10/24.
//

#ifndef ALBUMARCHITECT_SIMILARITY_SEARCH_H
#define ALBUMARCHITECT_SIMILARITY_SEARCH_H
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
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

  /// Returns all duplicated photo IDs
  /// @return
  auto get_duplicated_photos() -> std::vector<std::vector<PhotoId>>;

  /// Returns all the duplicates of a given photo (including itself)
  /// @param photo
  /// @return
  auto get_duplicates_of(album::Photo& photo) -> std::vector<PhotoId>;

private:
  std::unique_ptr<SimilarityIndex> m_similarity_index;
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
  SimilaritySearchBuilder(SimilaritySearchBuilder&& other) noexcept = default;
  auto operator=(const SimilaritySearchBuilder& other)
      -> SimilaritySearchBuilder& = delete;
  auto operator=(SimilaritySearchBuilder&& other) noexcept
      -> SimilaritySearchBuilder& = default;

  /// Adds a photo to the index builder and returns a unique ID
  /// @param photo
  /// @return
  auto add_photo(album::Photo& photo) -> PhotoId;

  auto build_search() -> SimilaritySearch;

private:
  std::unique_ptr<class SimilarityIndex> m_similarity_index;
  std::size_t m_current_id = 0;
};

}  // namespace album_architect::analysis

#endif  // ALBUMARCHITECT_SIMILARITY_SEARCH_H
