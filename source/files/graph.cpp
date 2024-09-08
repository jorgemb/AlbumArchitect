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
    m_graph[m_root_node].type = NodeType::directory;
  }
}
void FileGraph::add_node(boost::span<std::string> path_list, NodeType type) {
  // If the path list is empty do not create a new NodeId
  if (path_list.empty()) {
    return;
  }

  // Try to find the previous NodeId in the cache
  auto previous_node = m_root_node;
  if (path_list.size() > 1) {
    previous_node =
        get_or_create_nodes(path_list.subspan(0, path_list.size() - 1));
  }

  // Create the new NodeId
  // TODO: Check if NodeId already exists
  const auto new_node = boost::add_vertex({type}, m_graph);
  boost::add_edge(previous_node, new_node, path_list.back(), m_graph);
}
void FileGraph::to_graphviz(std::ostream& ostream) const {
  const auto edge_property = boost::get(boost::edge_name_t(), m_graph);
  boost::write_graphviz(
      ostream,
      m_graph,
      [this](std::ostream& out, const auto& vertex)
      { out << m_graph[vertex].type; },
      boost::make_label_writer(edge_property));
}
auto FileGraph::rename_node(boost::span<std::string> path_list,
                            const std::string& new_name) -> bool {
  // TODO: Fix cache, but clear for the time being
  m_vertex_cache.clear();

  // Root NodeId cannot be renamed
  if (path_list.empty()) {
    spdlog::error("Cannot rename root NodeId");
    return false;
  }

  // Get the NodeId data
  auto node_data = get_node_data(path_list);
  if (!node_data) {
    return false;
  }

  auto [edge, node] = *node_data;
  const auto name_property = boost::get(boost::edge_name_t(), m_graph);
  boost::put(name_property, edge, new_name);
  return true;
}
auto FileGraph::get_node_data(boost::span<const std::string> path_list)
    -> std::optional<
        std::pair<GraphType::edge_descriptor, GraphType::vertex_descriptor>> {
  // Search for the NodeId
  auto current_node = m_root_node;
  auto current_edge = GraphType::edge_descriptor {};
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
    -> std::optional<GraphType::vertex_descriptor> {
  const auto hash = boost::hash_value(path_list);
  const auto cache_iterator = m_vertex_cache.find(hash);
  if (cache_iterator != m_vertex_cache.end()) {
    return std::make_optional(cache_iterator->second);
  }
  return {};
}
void FileGraph::add_node_to_cache(boost::span<const std::string> path_list,
                                  GraphType::vertex_descriptor vertex) {
  auto hash = boost::hash_value(path_list);
  m_vertex_cache[hash] = vertex;
}
auto FileGraph::get_or_create_nodes(boost::span<const std::string> path_list)
    -> GraphType::vertex_descriptor {
  // Try to find in the cache
  const auto cached_node = get_node_from_cache(path_list);
  if (cached_node) {
    return cached_node.value();
  }

  // Iterate through the graph to find the NodeId, and create any intermediary
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

    // Create the new NodeId
    const auto new_node = boost::add_vertex({NodeType::directory}, m_graph);
    boost::add_edge(current_node, new_node, current_path, m_graph);
    current_node = new_node;

    // Add the NodeId to the cache
    add_node_to_cache(path_list, current_node);
  }

  return current_node;
}
auto FileGraph::operator==(const FileGraph& rhs) const -> bool {
  // Compare number of vertices and edges
  if (boost::num_vertices(m_graph) != boost::num_vertices(rhs.m_graph)) {
    return false;
  }

  if (boost::num_edges(m_graph) != boost::num_edges(rhs.m_graph)) {
    return false;
  }

  // TODO: Provide isomorphism equality. Right now is exact equality.
  auto [lhs_iter, lhs_e] = boost::vertices(m_graph);
  auto [rhs_iter, rhs_e] = boost::vertices(rhs.m_graph);
  for (; lhs_iter != lhs_e && rhs_iter != rhs_e; ++lhs_iter, ++rhs_iter) {
    if (m_graph[*lhs_iter] != rhs.m_graph[*rhs_iter]) {
      return false;
    }
  }

  return true;
}
auto FileGraph::operator!=(const FileGraph& rhs) const -> bool {
  return !(rhs == *this);
}
auto FileGraph::get_node(boost::span<const std::string> path_list)
    -> std::optional<NodeId> {
  // Check for the case where the node is the root
  if (path_list.empty()) {
    return {m_root_node};
  }

  if (path_list.size() == 1 && path_list[0] == ".") {
    return {m_root_node};
  }

  // Get the node and return
  auto node_data = get_node_data(path_list);
  if (!node_data) {
    return {};
  }
  auto [edge, vertex] = node_data.value();

  return std::make_optional<NodeId>(vertex);
}
auto FileGraph::get_node_type(FileGraph::NodeId node) -> NodeType {
  return m_graph[node].type;
}
auto FileGraph::get_node_children(FileGraph::NodeId node_id)
    -> std::vector<NodeId> {
  auto ret = std::vector<NodeId> {};
  auto [begin, end] = boost::out_edges(node_id, m_graph);

  // Get the target vertex of each edge
  std::transform(begin,
                 end,
                 std::back_inserter(ret),
                 [this](auto edge) { return boost::target(edge, m_graph); });

  return ret;
}
auto FileGraph::get_node_path(FileGraph::NodeId node_id)
    -> std::vector<std::string> {
  auto ret = std::vector<std::string> {};

  auto current_node = node_id;
  const auto name_property = boost::get(boost::edge_name_t(), m_graph);
  while (current_node != m_root_node) {
    auto [edges_b, edges_e] = boost::in_edges(current_node, m_graph);

    // NOTE: Except for the root, every vertex should have in-edges
    if (edges_b == edges_e) {
      const auto message = fmt::format(
          "The vertex with ID {} is not the root and doesn't have in-edges.",
          node_id);
      spdlog::error(message);
      throw std::runtime_error(message);
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
