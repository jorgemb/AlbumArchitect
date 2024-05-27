//
// Created by Jorge on 08/05/2024.
//

#include <algorithm>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "graph.h"

#include <boost/container_hash/hash.hpp>
#include <boost/core/span.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graphviz.hpp>
#include <spdlog/spdlog.h>

namespace album_architect::files {
FileGraph::FileGraph(bool create_root) {
  if (create_root) {
    m_root_node = boost::add_vertex(m_graph);
    auto color = boost::get(boost::vertex_color_t(), m_graph);
    boost::put(color, m_root_node, NodeType::directory);
  }
}
void FileGraph::add_node(boost::span<std::string> path_list, NodeType type) {
  // If the path list is empty do not create a new node_id
  if (path_list.empty()) {
    return;
  }

  // Try to find the previous node_id in the cache
  auto previous_node = m_root_node;
  if (path_list.size() > 1) {
    previous_node =
        get_or_create_nodes(path_list.subspan(0, path_list.size() - 1));
  }

  // Create the new node_id
  // TODO: Check if node_id already exists
  auto new_node = boost::add_vertex(type, m_graph);
  boost::add_edge(previous_node, new_node, path_list.back(), m_graph);
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
  // TODO: Fix cache, but clear for the time being
  m_vertex_cache.clear();

  // Root node_id cannot be renamed
  if (path_list.empty()) {
    spdlog::error("Cannot rename root node_id");
    return false;
  }

  // Get the node_id data
  auto node_data = get_node_data(path_list);
  if (!node_data) {
    return false;
  }

  auto [edge, node] = *node_data;
  auto name_property = boost::get(boost::edge_name_t(), m_graph);
  boost::put(name_property, edge, new_name);
  return true;
}
auto FileGraph::get_node_data(boost::span<const std::string> path_list)
    -> std::optional<
        std::pair<graph_type::edge_descriptor, graph_type::vertex_descriptor>> {
  // Search for the node_id
  auto current_node = m_root_node;
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
auto FileGraph::get_node_from_cache(boost::span<const std::string> path_list)
    -> std::optional<graph_type::vertex_descriptor> {
  auto hash = boost::hash_value(path_list);
  auto cache_iterator = m_vertex_cache.find(hash);
  if (cache_iterator != m_vertex_cache.end()) {
    return std::make_optional(cache_iterator->second);
  }
  return {};
}
void FileGraph::add_node_to_cache(boost::span<const std::string> path_list,
                                  graph_type::vertex_descriptor vertex) {
  auto hash = boost::hash_value(path_list);
  m_vertex_cache[hash] = vertex;
}
auto FileGraph::get_or_create_nodes(boost::span<const std::string> path_list)
    -> graph_type::vertex_descriptor {
  // Try to find in the cache
  auto cached_node = get_node_from_cache(path_list);
  if (cached_node) {
    return cached_node.value();
  }

  // Iterate through the graph to find the node_id, and create any intermediary
  // nodes as directories
  auto current_node = m_root_node;
  bool last_was_created =
      false;  // Set to true on the first edge that is created. Afterward every
              // edge should be created.
  auto name_property = boost::get(boost::edge_name_t(), m_graph);

  // Iterate through every member of the path list
  for (const auto& current_path : path_list) {
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

    // Create the new node_id
    auto new_node = boost::add_vertex(NodeType::directory, m_graph);
    boost::add_edge(current_node, new_node, current_path, m_graph);
    current_node = new_node;

    // Add the node_id to the cache
    add_node_to_cache(path_list, current_node);
  }

  return current_node;
}
bool FileGraph::operator==(const FileGraph& rhs) const {
  // Compare number of vertices and edges
  if (boost::num_vertices(m_graph) != boost::num_vertices(rhs.m_graph)) {
    return false;
  }

  if (boost::num_edges(m_graph) != boost::num_edges(rhs.m_graph)) {
    return false;
  }

  // TODO: Provide isomorphism equality. Right now is exact equality.
  auto lhs_type = boost::get(boost::vertex_color_t(), m_graph);
  auto rhs_type = boost::get(boost::vertex_color_t(), rhs.m_graph);
  auto [lhs_iter, lhs_e] = boost::vertices(m_graph);
  auto [rhs_iter, rhs_e] = boost::vertices(rhs.m_graph);
  for (; lhs_iter != lhs_e && rhs_iter != rhs_e; ++lhs_iter, ++rhs_iter) {
    if (boost::get(lhs_type, *lhs_iter) != boost::get(rhs_type, *rhs_iter)) {
      return false;
    }
  }

  return true;
}
bool FileGraph::operator!=(const FileGraph& rhs) const {
  return !(rhs == *this);
}
auto FileGraph::get_node(boost::span<const std::string> path_list)
    -> std::optional<node_id> {
  // Get the node and return
  auto node_data = get_node_data(path_list);
  if (!node_data) {
    return {};
  }
  auto [edge, vertex] = node_data.value();

  return std::make_optional<node_id>(vertex);
}
auto FileGraph::get_node_type(FileGraph::node_id node) -> NodeType {
  auto type_property = boost::get(boost::vertex_color_t(), m_graph);
  return boost::get(type_property, node);
}
auto FileGraph::get_node_children(FileGraph::node_id id)
    -> std::vector<node_id> {
  auto ret = std::vector<node_id> {};
  auto [begin, end] = boost::out_edges(id, m_graph);

  // Get the target vertex of each edge
  std::transform(begin,
                 end,
                 std::back_inserter(ret),
                 [this](auto edge) { return boost::target(edge, m_graph); });

  return ret;
}
auto FileGraph::get_node_path(FileGraph::node_id id)
    -> std::vector<std::string> {
  auto ret = std::vector<std::string>{};

  auto current_node = id;
  auto name_property = boost::get(boost::edge_name_t(), m_graph);
  while(current_node != m_root_node){
    auto [edges_b, edges_e] = boost::in_edges(current_node, m_graph);

    // NOTE: Except for the root, every vertex should have in-edges
    if(edges_b == edges_e){
      auto message = fmt::format("The vertex with ID {} is not the root and doesn't have in-edges.", id);
      spdlog::error(message);
      throw std::runtime_error(std::move(message));
    }

    auto edge = *edges_b;
    ret.push_back(boost::get(name_property, edge));
    current_node = boost::source(edge, m_graph);
  }

  // Reverse the path so it starts at the root
  std::reverse(ret.begin(), ret.end());

  return ret;
}

auto operator<<(std::ostream& ostream, const NodeType& node) -> std::ostream& {
  switch (node) {
    case NodeType::directory:
      ostream << "d";
      break;
    case NodeType::file:
      ostream << "f";
      break;
  }

  return ostream;
}
}  // namespace album_architect::files
