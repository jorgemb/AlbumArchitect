#ifndef ALBUMARCHITECT_BOOST_SERIALIZATION_CVMAT_H
#define ALBUMARCHITECT_BOOST_SERIALIZATION_CVMAT_H

#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>
#include <opencv2/opencv.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(::cv::Mat)

namespace boost::serialization {

/** Serialization support for cv::Mat */
template<class Archive>
void save(Archive& archive,
          const ::cv::Mat& mat,
          const unsigned int /*version*/) {
  size_t elem_size = mat.elemSize();
  int elem_type = mat.type();

  archive & mat.cols;
  archive & mat.rows;
  archive & elem_size;
  archive & elem_type;

  const size_t data_size = static_cast<size_t>(mat.cols * mat.rows) * elem_size;
  archive& boost::serialization::make_array(mat.ptr(), data_size);
}

/** Serialization support for cv::Mat */
template<class Archive>
void load(Archive& archive, ::cv::Mat& mat, const unsigned int /*version*/) {
  auto cols = int {};
  auto rows = int {};
  auto elem_type = int {};
  auto elem_size = size_t {};

  archive & cols;
  archive & rows;
  archive & elem_size;
  archive & elem_type;

  mat.create(rows, cols, elem_type);

  size_t data_size = static_cast<size_t>(mat.cols * mat.rows) * elem_size;
  archive& boost::serialization::make_array(mat.ptr(), data_size);
}

}  // namespace boost::serialization

#endif  // ALBUMARCHITECT_BOOST_SERIALIZATION_CVMAT_H
