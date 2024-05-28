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

/// Represents the type of node_id
enum class NodeType : std::uint8_t {
  directory,
  file
};

/// Print the node_id type to standard output
/// \param ostream
/// \param node
/// \return
auto operator<<(std::ostream& ostream, const NodeType& node) -> std::ostream&;

// Type used for representing the internal graph
using vertex_property = boost::property<boost::vertex_color_t, NodeType>;
using edge_property = boost::property<boost::edge_name_t, std::string>;

using graph_type = boost::adjacency_list<boost::vecS,
                                         boost::vecS,
                                         boost::bidirectionalS,
                                         vertex_property,
                                         edge_property>;

/// Represent a filesystem with graphs
class FileGraph {
public:
  using path_hash = std::size_t;
  using node_id = graph_type::vertex_descriptor;

  /// Creates the root node_id of the FileGraph
  explicit FileGraph(bool create_root = false);

  /// Creates a new node_id with all the intermediate representations
  /// \param path_list
  void add_node(boost::span<std::string> path_list, NodeType type);

  /// Returns the type of the node_id if it exists.
  /// \param path_list
  /// \return
  auto get_node_type(node_id node) -> NodeType;

  /// Returns the ID of the node in the given path
  /// \param path_list
  /// \return
  auto get_node(boost::span<const std::string> path_list) -> std::optional<node_id>;

  /// Returns the children of the given node
  /// \param id
  /// \return
  auto get_node_children(node_id id) -> std::vector<node_id>;

  /// Returns the path from a given node
  /// \param id
  /// \return
  auto get_node_path(node_id id) -> std::vector<std::string>;

  /// Changes the name of a node_id with the new name
  /// \param path_list
  /// \param new_name
  auto rename_node(boost::span<std::string> path_list,
                   const std::string& new_name) -> bool;

  /// Moves a node_id to a new path, conserving all sub-nodes
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
  void to_graphviz(std::ostream& os) const;

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
  bool operator==(const FileGraph& rhs) const;
  bool operator!=(const FileGraph& rhs) const;

private:
  /// Internal graph
  graph_type m_graph;
  graph_type::vertex_descriptor m_root_node;

  /// Cache for efficient lookup
  absl::btree_map<path_hash, graph_type::vertex_descriptor> m_vertex_cache;

  /// Returns a node_id with the edge that goes into it, if it exists
  /// \param path_list
  /// \return
  auto get_node_data(boost::span<const std::string> path_list)
      -> std::optional<std::pair<graph_type::edge_descriptor,
                                 graph_type::vertex_descriptor>>;

  /// Gets or creates the nodes in the path_list, and returns the last one
  /// \param path_list
  /// \return
  auto get_or_create_nodes(boost::span<const std::string> path_list)
      -> graph_type::vertex_descriptor;

  /// Returns a node_id from the cache if it exists
  /// \param path_list
  /// \return
  auto get_node_from_cache(boost::span<const std::string> path_list)
      -> std::optional<graph_type::vertex_descriptor>;

  /// Adds a node_id to the cache
  /// \param path_list
  /// \param vertex
  void add_node_to_cache(boost::span<const std::string> path_list,
                         graph_type::vertex_descriptor vertex);
};

}  // namespace album_architect::files

#endif  // ALBUMARCHITECT_GRAPH_H
