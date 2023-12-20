//
// Created by jorge on 12/20/23.
//

#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>
#include <fmt/core.h>
#include <opencv2/core.hpp>
#include <support/serialize/cvmat.h>

TEST_CASE("CV Mat serialization", "[support][serialize]") {
  constexpr int rows = 4;
  constexpr int cols = 12;

  auto random_mat = cv::Mat(rows, cols, CV_32FC1);
  const auto random_min = cv::Scalar(0);
  const auto random_max = cv::Scalar(255);

  cv::randu(random_mat, random_min, random_max);

  // Try to serialize
  auto stream = std::stringstream {};
  {
    auto output = cereal::BinaryOutputArchive{stream};
    output(random_mat);
  }

  // Try to load
  auto loaded_mat = cv::Mat {};
  {
    auto input = cereal::BinaryInputArchive(stream);
    input(loaded_mat);
  }

  // Check equality
  auto diff = cv::Mat {};
  cv::compare(random_mat, loaded_mat, diff, cv::CMP_NE);
  auto zeros = cv::countNonZero(diff);
  REQUIRE(zeros == 0);
}