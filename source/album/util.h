//
// Created by jorge on 30/10/2023.
//

#ifndef ALBUMARCHITECT_UTIL_H
#define ALBUMARCHITECT_UTIL_H

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

}  // namespace album_architect::util

#endif  // ALBUMARCHITECT_UTIL_H
