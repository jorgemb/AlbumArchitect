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
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>

namespace album_architect::files {

/// Represents the type of node
enum class NodeType : std::uint8_t {
  directory,
  file
};

/// Print the node type to standard output
/// \param ostream
/// \param node
/// \return
auto operator<<(std::ostream& ostream, const NodeType& node) -> std::ostream&;

class FileGraph {
public:
  using path_hash = std::size_t;

  /// Creates the root node of the FileGraph
  FileGraph();

  /// Creates a new node with all the intermediate representations
  /// \param path_list
  void add_node(boost::span<std::string> path_list, NodeType type);

  /// Returns the type of the node if it exists.
  /// \param path_list
  /// \return
  auto get_node_type(boost::span<std::string> path_list)
      -> std::optional<NodeType>;

  /// Changes the name of a node with the new name
  /// \param path_list
  /// \param new_name
  auto rename_node(boost::span<std::string> path_list,
                   const std::string& new_name) -> bool;

  /// Moves a node to a new path, conserving all sub-nodes
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

private:
  using vertex_property = boost::property<boost::vertex_color_t, NodeType>;
  using edge_property = boost::property<boost::edge_name_t, std::string>;

  /// Type used for representing the internal graph
  using graph_type = boost::adjacency_list<boost::vecS,
                                           boost::vecS,
                                           boost::directedS,
                                           vertex_property,
                                           edge_property>;

  /// Internal graph
  graph_type m_graph;
  graph_type::vertex_descriptor m_root_node;

  /// Cache for efficient lookup
  absl::btree_map<path_hash, graph_type::vertex_descriptor> m_vertex_cache;

  /// Returns a node with the edge that goes into it, if it exists
  /// \param path_list
  /// \return
  auto get_node_data(boost::span<std::string> path_list)
      -> std::optional<std::pair<graph_type::edge_descriptor,
                                 graph_type::vertex_descriptor>>;

  /// Gets or creates the nodes in the path_list, and returns the last one
  /// \param path_list
  /// \return
  auto get_or_create_nodes(boost::span<std::string> path_list) -> graph_type::vertex_descriptor;

  /// Returns a node from the cache if it exists
  /// \param path_list
  /// \return
  auto get_node_from_cache(boost::span<std::string> path_list)
      -> std::optional<graph_type::vertex_descriptor>;

  /// Adds a node to the cache
  /// \param path_list
  /// \param vertex
  void add_node_to_cache(boost::span<std::string> path_list, graph_type::vertex_descriptor vertex);
};

}  // namespace album_architect::files

#endif  // ALBUMARCHITECT_GRAPH_H
