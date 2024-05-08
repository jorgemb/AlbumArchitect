//
// Created by Jorge on 08/05/2024.
//

#ifndef ALBUMARCHITECT_GRAPH_H
#define ALBUMARCHITECT_GRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <filesystem>

using graph_type = boost::adjacency_list<>;

namespace album_architect::files {

class FileGraph{
public:
  FileGraph();
private:
  graph_type m_graph;
};

} // namespace album_architect::files

#endif  // ALBUMARCHITECT_GRAPH_H
