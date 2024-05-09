//
// Created by Jorge on 08/05/2024.
//

#ifndef ALBUMARCHITECT_GRAPH_H
#define ALBUMARCHITECT_GRAPH_H

#include <filesystem>
#include <string>

#include <boost/core/span.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace album_architect::files {

/// Represents the type of node
enum class NodeType {
  directory,
  file
};

/// Print the node type to standard output
/// \param os
/// \param node
/// \return
auto operator<<(std::ostream& os, const NodeType& node) -> std::ostream&;

class FileGraph {
public:
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
  graph_type::vertex_descriptor m_root_vertex;
};

}  // namespace album_architect::files

#endif  // ALBUMARCHITECT_GRAPH_H
