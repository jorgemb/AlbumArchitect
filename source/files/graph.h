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

/// Represents Vertex data that is stored next to the graph
class VertexData {
public:
  NodeType type;

  /// Allow for default comparison
  auto operator<=>(const VertexData& rhs) const = default;

  /// Serialize object
  template <class Archive>
  void serialize(Archive& archive, const unsigned int  /*version*/){
    archive & type;
  }
};

// Type used for representing the internal graph
using EdgeProperty = boost::property<boost::edge_name_t, std::string>;

using GraphType = boost::adjacency_list<boost::vecS,
                                        boost::vecS,
                                        boost::bidirectionalS,
                                        VertexData,
                                        EdgeProperty>;

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
