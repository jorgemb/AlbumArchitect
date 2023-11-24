//
// Created by jorge on 30/10/2023.
//

#ifndef ALBUMARCHITECT_UTIL_H
#define ALBUMARCHITECT_UTIL_H

#include <filesystem>
#include <utility>

namespace album_architect::util {

/// Allows handling OpenCV log messages and redirect them to glog. Use this
/// function as follows:
/// ```
/// cv::redirectError(handle_opencv_log_messages);
/// ```
/// \param status
/// \param err_msg
/// \param file_name
/// \param line
/// \return
auto handle_cv_log_messages(int status,
                            const char* func_name,
                            const char* err_msg,
                            const char* file_name,
                            int line,
                            void* userdata) -> int;

/// \brief Manages a self destructing temporary directory with a random name
class AutoTempDirectory {
  std::filesystem::path m_path;

  /// \brief Generates a random name valid for the filesystem
  /// \return
  static auto generate_random_name(size_t size = 16) -> std::string;

public:
  /* Operators: Copy deleted, move default */
  AutoTempDirectory(const AutoTempDirectory& other) = delete;
  AutoTempDirectory(AutoTempDirectory&& other) noexcept = default;
  auto operator=(const AutoTempDirectory& other) -> AutoTempDirectory& = delete;
  auto operator=(AutoTempDirectory&& other) noexcept
      -> AutoTempDirectory& = default;

  /// \brief Creates a new temporary directory with a random name
  AutoTempDirectory();

  /// \brief Returns the name of the current directory
  /// \return
  [[nodiscard]] auto name() const -> std::string {
    return m_path.filename().string();
  }

  /// \brief Returns the path to the temporary directory
  /// \return
  [[nodiscard]] auto path() const -> const std::filesystem::path& {
    return m_path;
  }

  /// \brief Deletes the temporary directory and all the contents
  ~AutoTempDirectory();
};

/// \brief Class that helps setting the working directory temporarily
/// with RAII lifeline
class AutoSetWorkingDirectory {
  std::filesystem::path m_previous_path;

public:
  /// \brief Sets the working directory to the given path
  /// \param path
  explicit AutoSetWorkingDirectory(const std::filesystem::path& path);

  /// \brief Restores the previous working directory
  ~AutoSetWorkingDirectory();
};

/// \brief Creates a test image with a checker pattern at the specified path
/// \param path
/// \param width
/// \param height
/// \return
auto create_test_image(const std::filesystem::path& path, int width, int height)
    -> bool;

}  // namespace album_architect::util

#endif  // ALBUMARCHITECT_UTIL_H
