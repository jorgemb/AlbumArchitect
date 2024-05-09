//
// Created by Jorge on 08/05/2024.
//

#include "graph.h"


namespace album_architect::files {
FileGraph::FileGraph(): m_root_vertex(boost::add_vertex(m_graph)){
}
} // namespace album_architect::files
