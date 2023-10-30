//
// Created by jorge on 30/10/2023.
//

#include "util.h"

#include <glog/logging.h>
#include <opencv2/core/utils/logger.defines.hpp>

namespace album_architect::util {
auto handle_cv_log_messages(int status,
                            const char* /*unused*/,
                            const char* err_msg,
                            const char* file_name,
                            int line,
                            void* /*unused*/) -> int {
  // Convert severity
  auto severity = google::GLOG_0;
  switch (status) {
    case CV_LOG_LEVEL_DEBUG:
    case CV_LOG_LEVEL_INFO:
      severity = google::GLOG_INFO;
      break;
    case CV_LOG_LEVEL_WARN:
      severity = google::GLOG_WARNING;
      break;
    case CV_LOG_LEVEL_ERROR:
      severity = google::GLOG_ERROR;
      break;
    case CV_LOG_LEVEL_FATAL:
      severity = google::GLOG_FATAL;
      break;
    default:
      severity = google::GLOG_INFO;
      break;
  }

  // Create message
  google::LogMessage(file_name, line, severity).stream() << err_msg;

  return 0;
}
}  // namespace album_architect::util
