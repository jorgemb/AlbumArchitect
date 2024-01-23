//
// Created by jorge on 12/20/23.
// Adds functionality for CV Mat serialization
//

#ifndef ALBUMARCHITECT_SUPPORT_CVMAT_H
#define ALBUMARCHITECT_SUPPORT_CVMAT_H

#include <opencv2/core/mat.hpp>
#include <cereal/archives/binary.hpp>

namespace cereal {
template<class Archive>
void serialize(Archive& archive, cv::Mat& mat) {
  int cols {};
  int rows {};
  int type {};
  bool continuous {};

  if (Archive::is_saving::value) {
    cols = mat.cols;
    rows = mat.rows;
    type = mat.type();
    continuous = mat.isContinuous();
  }

  archive(cols, rows, type, continuous);

  if (Archive::is_loading::value) {
    mat.create(rows, cols, type);
  }

  if (continuous) {
    const auto data_size = static_cast<size_t>(rows * cols) * mat.elemSize();
    archive(binary_data(mat.ptr(), data_size));
  } else {
    const auto row_size = static_cast<size_t>(cols) * mat.elemSize();
    for (int i = 0; i < rows; i++) {
      archive(binary_data(mat.ptr(i), row_size));
    }
  }
}
}  // namespace cereal

#endif  // ALBUMARCHITECT_SUPPORT_CVMAT_H
