//
// Created by Jorge on 08/05/2024.
//

#ifndef ALBUMARCHITECT_GRAPH_H
#define ALBUMARCHITECT_GRAPH_H

#include <filesystem>
#include <string>

#include <boost/graph/adjacency_list.hpp>

namespace album_architect::files {

class FileGraph {
public:
  /// Creates the root node of the FileGraph
  FileGraph();

private:
  using vertex_property = boost::property<boost::vertex_name_t, std::string>;
  /// Type used for representing the internal graph
  using graph_type =
      boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, vertex_property>;

  /// Internal graph
  graph_type m_graph;
  graph_type::vertex_descriptor m_root_vertex;
};

}  // namespace album_architect::files

#endif  // ALBUMARCHITECT_GRAPH_H
