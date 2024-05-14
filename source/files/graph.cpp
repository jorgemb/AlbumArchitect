//
// Created by Jorge on 08/05/2024.
//

#include <sstream>
#include <spdlog/spdlog.h>

#include "graph.h"

#include <boost/graph/graphviz.hpp>

namespace album_architect::files {
FileGraph::FileGraph()
    : m_root_vertex(boost::add_vertex(m_graph)) {
  auto color = boost::get(boost::vertex_color_t(), m_graph);
  boost::put(color, m_root_vertex, NodeType::directory);
}
auto FileGraph::get_node_type(boost::span<std::string> path_list)
    -> std::optional<NodeType> {
  auto current_node = m_root_vertex;
  auto name_property = boost::get(boost::edge_name_t(), m_graph);
  for (auto& current_path : path_list) {
    // Find if any of the outer edges has the given name
    auto [start, end] = boost::out_edges(current_node, m_graph);
    auto next_node = std::find_if(
        start,
        end,
        [&name_property, &current_path](auto iter)
        { return boost::get(name_property, iter) == current_path; });

    if (next_node != end) {
      current_node = boost::target(*next_node, m_graph);
    } else {
      return {};
    }
  }

  auto color_property = boost::get(boost::vertex_color_t(), m_graph);
  return std::make_optional<NodeType>(boost::get(color_property, current_node));
}
void FileGraph::add_node(boost::span<std::string> path_list, NodeType type) {
  auto current_node = m_root_vertex;
  bool last_was_created =
      false;  // Set to true on the first edge that is created. Afterward every
              // edge should be created.
  auto name_property = boost::get(boost::edge_name_t(), m_graph);
  for (const auto& current_path : path_list) {
    // Skip files that have "."
    if(current_path == "."){
      continue;
    }

    if (!last_was_created) {
      // Find among the nodes
      auto [start, end] = boost::out_edges(current_node, m_graph);
      auto found_edge = std::find_if(
          start,
          end,
          [&name_property, &current_path](auto iter)
          { return boost::get(name_property, iter) == current_path; });

      if (found_edge == end) {
        last_was_created = true;
      } else {
        current_node = boost::target(*found_edge, m_graph);
        continue;
      }
    }

    // Create the new node
    auto new_node = boost::add_vertex(NodeType::directory, m_graph);
    boost::add_edge(current_node, new_node, current_path, m_graph);
    current_node = new_node;
  }

  auto type_property = boost::get(boost::vertex_color_t(), m_graph);
  boost::put(type_property, current_node, type);
}
void FileGraph::to_graphviz(std::ostream& os) const {
  auto vertex_property = boost::get(boost::vertex_color_t(), m_graph);
  auto edge_property = boost::get(boost::edge_name_t(), m_graph);
  boost::write_graphviz(os,
                        m_graph,
                        boost::make_label_writer(vertex_property),
                        boost::make_label_writer(edge_property));
}
auto FileGraph::rename_node(boost::span<std::string> path_list,
                            const std::string& new_name) -> bool {
  // Root node cannot be renamed
  if(path_list.empty()){
    spdlog::error("Cannot rename root node");
    return false;
  }

  // Get the node data
  auto node_data = get_node_data(path_list);
  if (!node_data) {
    return false;
  }

  auto [edge, node] = *node_data;
  auto name_property = boost::get(boost::edge_name_t(), m_graph);
  boost::put(name_property, edge, new_name);
  return true;
}
auto FileGraph::get_node_data(boost::span<std::string> path_list)
    -> std::optional<
        std::pair<graph_type::edge_descriptor, graph_type::vertex_descriptor>> {
  // Search for the node
  auto current_node = m_root_vertex;
  auto current_edge = graph_type::edge_descriptor {};
  auto name_property = boost::get(boost::edge_name_t(), m_graph);

  for (const auto& current_path : path_list) {
    auto [start, end] = boost::out_edges(current_node, m_graph);
    auto edge_iter = std::find_if(
        start,
        end,
        [&name_property, &current_path](auto iter)
        { return boost::get(name_property, iter) == current_path; });

    if (edge_iter == end) {
      // The path was not found
      return {};
    }

    // Get the current info
    current_edge = *edge_iter;
    current_node = boost::target(*edge_iter, m_graph);
  }

  return std::make_pair(current_edge, current_node);
}
auto operator<<(std::ostream& ostream, const NodeType& node) -> std::ostream& {
  switch (node) {
    case NodeType::directory:
      ostream << "directory";
      break;
    case NodeType::file:
      ostream << "file";
      break;
  }

  return ostream;
}
}  // namespace album_architect::files
