#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace gjp {

using NodeId = std::uint32_t;
constexpr NodeId kInvalidNode = std::numeric_limits<NodeId>::max();

struct Coordinate {
    double lon{0.0};
    double lat{0.0};

    bool operator==(const Coordinate& o) const noexcept {
        return lon == o.lon && lat == o.lat;
    }
};

struct Edge {
    NodeId to{kInvalidNode};
    double weight{0.0};
};

struct Node {
    NodeId id{kInvalidNode};
    Coordinate coord{};
    std::string name{};
};

}  // namespace gjp
