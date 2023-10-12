//
// Created by jorge on 11/10/2023.
//

#ifndef ALBUMARCHITECT_ALBUM_EXCEPTION_H
#define ALBUMARCHITECT_ALBUM_EXCEPTION_H

#include <exception>

namespace album_architect {

class AlbumException : public std::exception {
public:
  explicit AlbumException(const char* const message)
      : exception(message) {}
};

}  // namespace album_architect

#endif  // ALBUMARCHITECT_ALBUM_EXCEPTION_H
