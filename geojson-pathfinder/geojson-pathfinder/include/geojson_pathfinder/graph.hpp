#pragma once

#include "geojson_pathfinder/types.hpp"

#include <cstddef>
#include <vector>

namespace gjp {

// Adjacency-list graph with contiguous storage.
// Node IDs are dense indices [0, num_nodes).
class Graph {
public:
    Graph() = default;

    // Add a node and return its assigned id.
    NodeId add_node(const Coordinate& coord, std::string name = {});

    // Add an undirected edge with given weight.
    void add_edge(NodeId a, NodeId b, double weight);

    // Add a directed edge with given weight.
    void add_directed_edge(NodeId from, NodeId to, double weight);

    [[nodiscard]] std::size_t num_nodes() const noexcept { return nodes_.size(); }
    [[nodiscard]] std::size_t num_edges() const noexcept { return num_edges_; }

    [[nodiscard]] const Node& node(NodeId id) const { return nodes_.at(id); }
    [[nodiscard]] const std::vector<Edge>& neighbors(NodeId id) const {
        return adjacency_.at(id);
    }

    [[nodiscard]] const std::vector<Node>& nodes() const noexcept { return nodes_; }

    // Distance helper: great-circle (haversine) in meters.
    static double haversine_meters(const Coordinate& a, const Coordinate& b) noexcept;

private:
    std::vector<Node> nodes_{};
    std::vector<std::vector<Edge>> adjacency_{};
    std::size_t num_edges_{0};
};

}  // namespace gjp
