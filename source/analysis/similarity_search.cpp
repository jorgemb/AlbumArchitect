//
// Created by jorge on 09/10/24.
//

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <ranges>
#include <utility>

#include "similarity_search.h"

#include <spdlog/spdlog.h>

#include "helper/cv_mat_operations.h"

// NOLINTBEGIN(*)
#define ANNOYLIB_MULTITHREADED_BUILD ;
#include <annoy/annoylib.h>
#include <annoy/kissrandom.h>
// NOLINTEND(*)

namespace album_architect::analysis {

namespace rng = std::ranges;

/// Hash ID
/// @tparam HashType
/// @tparam IdType
template<class HashType, class IdType = std::size_t>
struct HashId {
  /// Default constructor
  /// @param hash
  /// @param id
  HashId(const HashType& hash, const IdType& id)
      : hash(hash)
      , id(id) {}

  HashType hash;
  IdType id;
};

/// Contains the indices for comparing similarity
class SimilarityIndex {
public:
  /// Default constructor
  SimilarityIndex()
      : p_hash_index(8) {}

  /// Default destructor
  ~SimilarityIndex() = default;

  /// Index for pHash algorithm
  Annoy::AnnoyIndex<PhotoId,
                    uchar,
                    Annoy::Hamming,
                    Annoy::Kiss32Random,
                    Annoy::AnnoyIndexMultiThreadedBuildPolicy>
      p_hash_index;

  // Index for AverageSearch
  std::vector<HashId<std::uint64_t>> average_index;
};

SimilaritySearchBuilder::SimilaritySearchBuilder()
    : m_similarity_index(std::make_unique<SimilarityIndex>()) {}
SimilaritySearchBuilder::~SimilaritySearchBuilder() = default;
auto SimilaritySearchBuilder::add_photo(album::Photo& photo) -> PhotoId {
  // Add to the indices
  auto photo_id = m_current_id;
  ++m_current_id;

  // Add pHash
  const auto p_hash = photo.get_image_hash(album::ImageHashAlgorithm::p_hash);
  m_similarity_index->p_hash_index.add_item(photo_id, p_hash.data);

  // Add average hash
  const auto average_hash =
      photo.get_image_hash(album::ImageHashAlgorithm::average_hash);
  m_similarity_index->average_index.emplace_back(
      cvmat::mat_to_uint64(average_hash), photo_id);

  return photo_id;
}
auto SimilaritySearchBuilder::build_search() -> SimilaritySearch {
  // PHash build
  constexpr auto p_hash_trees = 2 * 8;  // Twice the size of each matrix(8)
  char* error = nullptr;
  if (!m_similarity_index->p_hash_index.build(p_hash_trees, -1, &error)) {
    spdlog::error("Couldn't build similarity index. Error: {}", error);
    free(error);
  }

  // AverageHash build
  rng::sort(m_similarity_index->average_index,
            [](const auto& lhs, const auto& rhs)
            { return lhs.hash < rhs.hash; });

  return SimilaritySearch(std::move(m_similarity_index));
}
SimilaritySearch::SimilaritySearch(
    std::unique_ptr<SimilarityIndex> similarity_index)
    : m_similarity_index(std::move(similarity_index)) {}
SimilaritySearch::~SimilaritySearch() = default;
auto SimilaritySearch::get_duplicated_photos() const
    -> std::vector<std::vector<PhotoId>> {
  // Check base case
  if (m_similarity_index->average_index.size() <= 1) {
    return {};
  }

  // Check for duplicates
  auto result = std::vector<std::vector<PhotoId>> {};
  auto current_duplicate_list = std::vector<PhotoId> {};

  for (auto current = std::next(m_similarity_index->average_index.begin());
       current != m_similarity_index->average_index.end();
       ++current)
  {
    // Check if this is duplicate of the previous
    auto previous = std::prev(current);
    if (current->hash == previous->hash) {
      // Duplicates
      if (current_duplicate_list.empty()) {
        current_duplicate_list.push_back(previous->id);
      }
      current_duplicate_list.push_back(current->id);
    } else {
      // Not duplicates
      if (!current_duplicate_list.empty()) {
        // Move current list of duplicates to the list, create a new one
        result.push_back(std::move(current_duplicate_list));
        current_duplicate_list = {};
      }
    }
  }

  // Check if last was missing
  if (!current_duplicate_list.empty()) {
    result.push_back(std::move(current_duplicate_list));
  }

  return result;
}
auto SimilaritySearch::get_duplicates_of(album::Photo& photo) const
    -> std::vector<PhotoId> {
  // Calculate hash
  const auto photo_hash = cvmat::mat_to_uint64(
      photo.get_image_hash(album::ImageHashAlgorithm::average_hash));

  // Find where the image starts
  const auto start = std::find_if(m_similarity_index->average_index.begin(),
                                  m_similarity_index->average_index.end(),
                                  [&photo_hash](const auto& current)
                                  { return current.hash == photo_hash; });

  if (start == m_similarity_index->average_index.end()) {
    // Nothing was found
    return {};
  }

  // Find where the duplicates end
  const auto end = std::find_if(start,
                                m_similarity_index->average_index.end(),
                                [&photo_hash](const auto& current)
                                { return current.hash != photo_hash; });

  auto result = std::vector<PhotoId> {};
  std::transform(start,
                 end,
                 std::back_inserter(result),
                 [](const auto& current) { return current.id; });
  return result;
}
auto SimilaritySearch::get_similars_of(album::Photo& photo,
                                       float similarity_threshold,
                                       std::size_t max_photos) const
    -> std::vector<std::pair<PhotoId, std::uint8_t>> {
  // Get the hash and find similar
  const auto photo_hash =
      photo.get_image_hash(album::ImageHashAlgorithm::p_hash);

  std::vector<PhotoId> similar_photos;
  std::vector<std::uint8_t> distances;
  m_similarity_index->p_hash_index.get_nns_by_vector(
      photo_hash.data, max_photos, -1, &similar_photos, &distances);

  // Get the list of elements
  std::vector<std::pair<PhotoId, std::uint8_t>> result;
  result.reserve(distances.size());

  std::transform(similar_photos.begin(),
                 similar_photos.end(),
                 distances.begin(),
                 std::back_inserter(result),
                 [](const auto& photo_id, const auto& distance)
                 { return std::make_pair(photo_id, distance); });

  // Remove photos under threshold
  constexpr auto max_bits =
      static_cast<float>(std::numeric_limits<std::uint64_t>::digits);
  const auto erase_start = std::remove_if(
      result.begin(),
      result.end(),
      [&similarity_threshold, &max_bits](const auto& photo_pair)
      {
        auto similarity = (max_bits - photo_pair.second) / max_bits;
        return similarity <= similarity_threshold;
      });
  result.erase(erase_start, result.end());

  return result;
}
}  // namespace album_architect::analysis