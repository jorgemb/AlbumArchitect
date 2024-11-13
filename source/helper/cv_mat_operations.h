#ifndef ALBUMARCHITECT_CV_MAT_OPERATIONS_H
#define ALBUMARCHITECT_CV_MAT_OPERATIONS_H

#include <cstdint>

#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>

namespace album_architect::cvmat {

/// Checks if two arrays have equal values
/// \param lhs
/// \param rhs
/// \return
inline auto compare_mat(const cv::Mat& lhs, const cv::Mat& rhs) -> bool {
  // Check basic params
  if (lhs.size() != rhs.size()) {
    return false;
  }

  if (lhs.type() != rhs.type()) {
    return false;
  }

  // Check values using XOR (if matrices are equal then all values should be
  // zero)
  auto diff = cv::Mat {};
  cv::bitwise_xor(lhs, rhs, diff);
  auto equal = cv::countNonZero(diff) == 0;
  return equal;
}

/// Converts a Matrix that has type UCHAR and 8 bytes to a 64bit
/// @param mat
/// @return
inline auto mat_to_uint64(cv::Mat mat) -> std::uint64_t {
  // Check if the input mat is of the correct size and type
  constexpr auto max_size = 8;
  CV_Assert(mat.total() == max_size && mat.type() == CV_8UC1);

  std::uint64_t result = 0;

  // Iterate over the 8 bytes and shift them into the result
  for (auto i = 0U; i < max_size; ++i) {
    // NOLINTNEXTLINE(*-magic-numbers,*-narrowing-conversions)
    result |= static_cast<std::uint64_t>(mat.at<uint8_t>(i)) << (8U * (7U - i));
  }

  return result;
}

}  // namespace album_architect::cvmat

#endif  // ALBUMARCHITECT_CV_MAT_OPERATIONS_H
