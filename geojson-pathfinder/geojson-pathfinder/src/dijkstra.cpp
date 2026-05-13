#include "geojson_pathfinder/dijkstra.hpp"

#include "geojson_pathfinder/priority_queue.hpp"

#include <algorithm>
#include <limits>

namespace gjp {

namespace {
constexpr double kInf = std::numeric_limits<double>::infinity();
}

ShortestPathResult Dijkstra::shortest_path(NodeId source, NodeId target) const {
    const std::size_t n = graph_.num_nodes();
    if (source >= n || target >= n) return {};

    std::vector<double> dist(n, kInf);
    std::vector<NodeId> prev(n, kInvalidNode);
    IndexedMinHeap heap(n);

    dist[source] = 0.0;
    heap.push(source, 0.0);

    while (!heap.empty()) {
        const auto [u, du] = heap.pop_min();
        if (u == target) break;

        for (const Edge& e : graph_.neighbors(u)) {
            const double alt = du + e.weight;
            if (alt < dist[e.to]) {
                dist[e.to] = alt;
                prev[e.to] = u;
                if (heap.contains(e.to)) {
                    heap.decrease_key(e.to, alt);
                } else {
                    heap.push(e.to, alt);
                }
            }
        }
    }

    if (dist[target] == kInf) return {};

    // Reconstruct path source -> target.
    std::vector<NodeId> path;
    for (NodeId cur = target; cur != kInvalidNode; cur = prev[cur]) {
        path.push_back(cur);
        if (cur == source) break;
    }
    std::reverse(path.begin(), path.end());
    return {true, dist[target], std::move(path)};
}

std::vector<double> Dijkstra::shortest_distances(NodeId source) const {
    const std::size_t n = graph_.num_nodes();
    std::vector<double> dist(n, kInf);
    if (source >= n) return dist;

    IndexedMinHeap heap(n);
    dist[source] = 0.0;
    heap.push(source, 0.0);

    while (!heap.empty()) {
        const auto [u, du] = heap.pop_min();
        for (const Edge& e : graph_.neighbors(u)) {
            const double alt = du + e.weight;
            if (alt < dist[e.to]) {
                dist[e.to] = alt;
                if (heap.contains(e.to)) {
                    heap.decrease_key(e.to, alt);
                } else {
                    heap.push(e.to, alt);
                }
            }
        }
    }
    return dist;
}

}  // namespace gjp
