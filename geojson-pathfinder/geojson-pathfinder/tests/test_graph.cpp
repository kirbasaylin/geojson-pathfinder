#include "geojson_pathfinder/graph.hpp"

#include <gtest/gtest.h>

using namespace gjp;

TEST(Graph, AddsNodes) {
    Graph g;
    auto a = g.add_node({0.0, 0.0}, "A");
    auto b = g.add_node({1.0, 0.0}, "B");
    EXPECT_EQ(g.num_nodes(), 2u);
    EXPECT_EQ(g.node(a).name, "A");
    EXPECT_EQ(g.node(b).coord.lon, 1.0);
}

TEST(Graph, UndirectedEdgeAddsBothDirections) {
    Graph g;
    auto a = g.add_node({0, 0});
    auto b = g.add_node({1, 0});
    g.add_edge(a, b, 5.0);
    EXPECT_EQ(g.neighbors(a).size(), 1u);
    EXPECT_EQ(g.neighbors(b).size(), 1u);
    EXPECT_DOUBLE_EQ(g.neighbors(a)[0].weight, 5.0);
    EXPECT_EQ(g.num_edges(), 2u);
}

TEST(Graph, DirectedEdgeOnlyOneDirection) {
    Graph g;
    auto a = g.add_node({0, 0});
    auto b = g.add_node({1, 0});
    g.add_directed_edge(a, b, 3.0);
    EXPECT_EQ(g.neighbors(a).size(), 1u);
    EXPECT_EQ(g.neighbors(b).size(), 0u);
}

TEST(Graph, HaversineDistance) {
    // London (-0.1276, 51.5074) <-> Paris (2.3522, 48.8566) ~= 343.5 km
    const double d = Graph::haversine_meters({-0.1276, 51.5074}, {2.3522, 48.8566});
    EXPECT_NEAR(d, 343'500.0, 5'000.0);
}

TEST(Graph, HaversineZeroForSamePoint) {
    EXPECT_DOUBLE_EQ(Graph::haversine_meters({10.0, 10.0}, {10.0, 10.0}), 0.0);
}
