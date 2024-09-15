//
// Created by Jorge on 08/05/2024.
//

#ifndef ALBUMARCHITECT_GRAPH_H
#define ALBUMARCHITECT_GRAPH_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <utility>

#include <absl/container/btree_map.h>
#include <boost/core/span.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <helper/boost_serialization_cvmat.h>
#include <opencv2/core.hpp>

namespace album_architect::files {

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

/// Represents Vertex data that is stored next to the graph
class VertexData {
public:
  /// Initialization constructor
  explicit VertexData(
      NodeType type,
      std::map<std::string, VertexAttribute> attributes = {})
      : type(type)
      , attributes(std::move(attributes)) {}

  /// Default constructor for auto insertion
  VertexData() = default;

  NodeType type = NodeType::directory;
  std::map<std::string, VertexAttribute> attributes;

  /// Serialize object
  template<class Archive>
  void serialize(Archive& archive, const unsigned int /*version*/) {
    archive & type;
    archive & attributes;
  }
};

/// Represents elements associated to edges
class EdgeData {
public:
  std::string name;

  /// Initialization constructor
  explicit EdgeData(std::string name);

  /// Default constructor
  EdgeData() = default;

  /// Serialize object
  template<class Archive>
  void serialize(Archive& archive, const unsigned int /*version*/) {
    archive & name;
  }
};

// Type used for representing the internal graph
using GraphType = boost::adjacency_list<boost::vecS,
                                        boost::vecS,
                                        boost::bidirectionalS,
                                        VertexData,
                                        EdgeData>;

/// Represent a filesystem with graphs
class FileGraph {
public:
  using PathHash = std::size_t;
  using NodeId = GraphType::vertex_descriptor;

  /// Creates the root NodeId of the FileGraph
  explicit FileGraph(bool create_root = false);

  /// Creates a new NodeId with all the intermediate representations
  /// \param path_list
  void add_node(boost::span<std::string> path_list, NodeType type);

  /// Returns the type of the NodeId if it exists.
  /// \param path_list
  /// \return
  auto get_node_type(NodeId node) -> NodeType;

  /// Returns the ID of the node in the given path
  /// \param path_list
  /// \return
  auto get_node(boost::span<const std::string> path_list)
      -> std::optional<NodeId>;

  /// Sets metadata for the given node
  /// \param node
  /// \param key Key to store the attribute with
  /// \param attribute Attribute to add to the node
  /// \return Previous attribute if any
  auto set_node_metadata(NodeId node, std::string_view key, const VertexAttribute& attribute) -> std::optional<VertexAttribute>;

  /// Returns the metadata for the given node, if any
  /// \param node
  /// \param key Key to get the value from
  /// \return Optional with a copy of the attribute
  auto get_node_metadata(NodeId node, std::string_view key) -> std::optional<VertexAttribute>;

  /// Removes the metadata from the node with the given key
  /// \param node
  /// \param key Key to get the value from
  /// \return Optional with the removed attribute
  auto remove_node_metadata(NodeId node, std::string_view key) -> std::optional<VertexAttribute>;

  /// Returns the children of the given node
  /// \param node_id
  /// \return
  auto get_node_children(NodeId node_id) -> std::vector<NodeId>;

  /// Returns the path from a given node
  /// \param node_id
  /// \return
  auto get_node_path(NodeId node_id) -> std::vector<std::string>;

  /// Changes the name of a NodeId with the new name
  /// \param path_list
  /// \param new_name
  auto rename_node(boost::span<std::string> path_list,
                   const std::string& new_name) -> bool;

  /// Moves a NodeId to a new path, conserving all sub-nodes
  /// TODO: Added at a later time, as it requires a merge operation
  /// \param old_path
  /// \param new_path
  /// \param force
  /// \return
  auto move_node(boost::span<std::string> old_path,
                 boost::span<std::string> new_path,
                 bool force = false) -> bool;

  /// Returns the GraphViz representation of the map. For debugging purposes.
  /// \return
  void to_graphviz(std::ostream& ostream) const;

  /// Enable serialization for the graph
  /// \tparam Archive
  /// \param archive
  /// \param version
  template<class Archive>
  void serialize(Archive& archive, unsigned int /* version */) {
    archive & m_graph;
    archive & m_root_node;
  }

  /// Comparison operators
  auto operator==(const FileGraph& rhs) const -> bool;
  auto operator!=(const FileGraph& rhs) const -> bool;

private:
  /// Internal graph
  GraphType m_graph;
  GraphType::vertex_descriptor m_root_node;

  /// Cache for efficient lookup
  absl::btree_map<PathHash, GraphType::vertex_descriptor> m_vertex_cache;

  /// Returns a NodeId with the edge that goes into it, if it exists
  /// \param path_list
  /// \return
  auto get_node_data(boost::span<const std::string> path_list)
      -> std::optional<
          std::pair<GraphType::edge_descriptor, GraphType::vertex_descriptor>>;

  /// Gets or creates the nodes in the path_list, and returns the last one
  /// \param path_list
  /// \return
  auto get_or_create_nodes(boost::span<const std::string> path_list)
      -> GraphType::vertex_descriptor;

  /// Returns a NodeId from the cache if it exists
  /// \param path_list
  /// \return
  auto get_node_from_cache(boost::span<const std::string> path_list)
      -> std::optional<GraphType::vertex_descriptor>;

  /// Adds a NodeId to the cache
  /// \param path_list
  /// \param vertex
  void add_node_to_cache(boost::span<const std::string> path_list,
                         GraphType::vertex_descriptor vertex);
};

}  // namespace album_architect::files

#endif  // ALBUMARCHITECT_GRAPH_H
