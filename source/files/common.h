#ifndef ALBUMARCHITECT_FILES_COMMON_H
#define ALBUMARCHITECT_FILES_COMMON_H

#include <ostream>
#include <variant>
#include <opencv2/core/mat.hpp>

namespace album_architect::files {
/// Represents the type of path
enum class PathType : std::uint8_t {
  invalid = 0,
  file,
  directory
};

/// Represents the type of NodeId
enum class NodeType : std::uint8_t {
  directory,
  file
};

/// Print the NodeId type to standard output
/// \param ostream
/// \param node
/// \return
auto operator<<(std::ostream& ostream, const NodeType& node) -> std::ostream&;

/// Represents a vertex attribute that can be stored along with each vertex
using VertexAttribute = std::variant<std::string, cv::Mat>;
using PathAttribute = VertexAttribute;

}  // namespace album_architect::files

#endif  // ALBUMARCHITECT_FILES_COMMON_H
