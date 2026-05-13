#include "geojson_pathfinder/dijkstra.hpp"
#include "geojson_pathfinder/graph.hpp"

#include <gtest/gtest.h>

#include <cmath>

using namespace gjp;

TEST(Dijkstra, SimpleTriangleShortcut) {
    // 0 --2-- 1 --2-- 2
    //  \_____ 5 _____/
    Graph g;
    auto a = g.add_node({0, 0});
    auto b = g.add_node({1, 0});
    auto c = g.add_node({2, 0});
    g.add_edge(a, b, 2.0);
    g.add_edge(b, c, 2.0);
    g.add_edge(a, c, 5.0);

    Dijkstra d(g);
    auto r = d.shortest_path(a, c);
    ASSERT_TRUE(r.found);
    EXPECT_DOUBLE_EQ(r.total_distance, 4.0);
    EXPECT_EQ(r.path.size(), 3u);
    EXPECT_EQ(r.path.front(), a);
    EXPECT_EQ(r.path.back(), c);
}

TEST(Dijkstra, NoPathWhenDisconnected) {
    Graph g;
    g.add_node({0, 0});
    g.add_node({1, 0});
    Dijkstra d(g);
    auto r = d.shortest_path(0, 1);
    EXPECT_FALSE(r.found);
    EXPECT_TRUE(r.path.empty());
}

TEST(Dijkstra, SourceEqualsTarget) {
    Graph g;
    g.add_node({0, 0});
    Dijkstra d(g);
    auto r = d.shortest_path(0, 0);
    ASSERT_TRUE(r.found);
    EXPECT_DOUBLE_EQ(r.total_distance, 0.0);
    EXPECT_EQ(r.path.size(), 1u);
}

TEST(Dijkstra, GridFindsManhattanLikePath) {
    // 3x3 grid with unit-weight edges. Expected SP from (0,0) to (2,2) = 4.
    Graph g;
    constexpr int N = 3;
    NodeId ids[N][N];
    for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x)
            ids[y][x] = g.add_node({static_cast<double>(x), static_cast<double>(y)});
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            if (x + 1 < N) g.add_edge(ids[y][x], ids[y][x + 1], 1.0);
            if (y + 1 < N) g.add_edge(ids[y][x], ids[y + 1][x], 1.0);
        }
    }
    Dijkstra d(g);
    auto r = d.shortest_path(ids[0][0], ids[2][2]);
    ASSERT_TRUE(r.found);
    EXPECT_DOUBLE_EQ(r.total_distance, 4.0);
    EXPECT_EQ(r.path.size(), 5u);
}

TEST(Dijkstra, ShortestDistancesFillsAllReachable) {
    Graph g;
    auto a = g.add_node({0, 0});
    auto b = g.add_node({1, 0});
    auto c = g.add_node({2, 0});
    auto d_node = g.add_node({3, 0});  // disconnected
    g.add_edge(a, b, 1.0);
    g.add_edge(b, c, 2.0);
    (void)d_node;

    Dijkstra solver(g);
    auto dists = solver.shortest_distances(a);
    ASSERT_EQ(dists.size(), 4u);
    EXPECT_DOUBLE_EQ(dists[a], 0.0);
    EXPECT_DOUBLE_EQ(dists[b], 1.0);
    EXPECT_DOUBLE_EQ(dists[c], 3.0);
    EXPECT_TRUE(std::isinf(dists[3]));
}
