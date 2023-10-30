//
// Created by jorge on 25/10/2023.
//

#ifndef ALBUMARCHITECT_TEXT_CLASSIFIER_H
#define ALBUMARCHITECT_TEXT_CLASSIFIER_H

#include <memory>
#include <string>

#include <tesseract/baseapi.h>

namespace album_architect {

class TextClassifier {
public:
  /// Returns a TesseractOCR classifier
  /// \return
  static auto get_tesseract_classifier(const std::string& language = "eng")
      -> std::unique_ptr<tesseract::TessBaseAPI>;
};

}  // namespace album_architect

#endif  // ALBUMARCHITECT_TEXT_CLASSIFIER_H
