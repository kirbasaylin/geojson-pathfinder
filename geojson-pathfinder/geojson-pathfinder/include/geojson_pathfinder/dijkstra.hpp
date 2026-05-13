#pragma once

#include "geojson_pathfinder/graph.hpp"
#include "geojson_pathfinder/types.hpp"

#include <optional>
#include <vector>

namespace gjp {

struct ShortestPathResult {
    bool found{false};
    double total_distance{0.0};
    std::vector<NodeId> path{};  // empty if not found; otherwise [source, ..., target]
};

// Dijkstra's algorithm with an indexed binary min-heap.
// Time:  O((V + E) log V)
// Space: O(V)
class Dijkstra {
public:
    explicit Dijkstra(const Graph& graph) : graph_(graph) {}

    // Compute shortest path from source to target.
    [[nodiscard]] ShortestPathResult shortest_path(NodeId source, NodeId target) const;

    // Compute single-source shortest distances to all reachable nodes.
    [[nodiscard]] std::vector<double> shortest_distances(NodeId source) const;

private:
    const Graph& graph_;
};

}  // namespace gjp
