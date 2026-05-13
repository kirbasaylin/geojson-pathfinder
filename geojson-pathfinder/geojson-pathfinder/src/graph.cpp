#include "geojson_pathfinder/graph.hpp"

#include <cmath>

namespace gjp {

NodeId Graph::add_node(const Coordinate& coord, std::string name) {
    const auto id = static_cast<NodeId>(nodes_.size());
    nodes_.push_back(Node{id, coord, std::move(name)});
    adjacency_.emplace_back();
    return id;
}

void Graph::add_edge(NodeId a, NodeId b, double weight) {
    adjacency_.at(a).push_back(Edge{b, weight});
    adjacency_.at(b).push_back(Edge{a, weight});
    num_edges_ += 2;
}

void Graph::add_directed_edge(NodeId from, NodeId to, double weight) {
    adjacency_.at(from).push_back(Edge{to, weight});
    ++num_edges_;
}

double Graph::haversine_meters(const Coordinate& a, const Coordinate& b) noexcept {
    constexpr double kEarthRadiusM = 6'371'000.0;
    constexpr double kPi = 3.14159265358979323846;
    const auto to_rad = [](double deg) { return deg * kPi / 180.0; };

    const double lat1 = to_rad(a.lat);
    const double lat2 = to_rad(b.lat);
    const double dlat = lat2 - lat1;
    const double dlon = to_rad(b.lon - a.lon);

    const double s = std::sin(dlat / 2.0);
    const double t = std::sin(dlon / 2.0);
    const double h = s * s + std::cos(lat1) * std::cos(lat2) * t * t;
    return 2.0 * kEarthRadiusM * std::asin(std::sqrt(h));
}

}  // namespace gjp
