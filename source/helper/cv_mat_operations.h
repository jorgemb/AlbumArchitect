#ifndef ALBUMARCHITECT_CV_MAT_OPERATIONS_H
#define ALBUMARCHITECT_CV_MAT_OPERATIONS_H

#include <opencv2/core.hpp>

namespace album_architect::cvmat {

/// Checks if two arrays have equal values
/// \param lhs
/// \param rhs
/// \return
inline bool compare_mat(cv::InputArray lhs, cv::InputArray rhs) {
  // Check basic params
  if (lhs.size() != rhs.size()) {
    return false;
  }

  if (lhs.type() != rhs.type()) {
    return false;
  }

  // Check values using XOR (if matrices are equal then all values should be zero)
  auto diff = cv::Mat {};
  cv::bitwise_xor(lhs, rhs, diff);
  auto equal = cv::countNonZero(diff) == 0;
  return equal;
}

}  // namespace album_architect::cvmat

#endif  // ALBUMARCHITECT_CV_MAT_OPERATIONS_H
