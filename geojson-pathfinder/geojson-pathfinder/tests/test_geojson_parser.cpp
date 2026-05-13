#include "geojson_pathfinder/geojson_parser.hpp"

#include <gtest/gtest.h>

using namespace gjp;

TEST(GeoJsonParser, ParsesEmptyFeatureCollection) {
    const std::string text = R"({"type":"FeatureCollection","features":[]})";
    auto g = GeoJsonParser::parse(text);
    EXPECT_EQ(g.num_nodes(), 0u);
}

TEST(GeoJsonParser, ParsesSinglePoint) {
    const std::string text = R"({
      "type":"FeatureCollection",
      "features":[
        {"type":"Feature","properties":{},
         "geometry":{"type":"Point","coordinates":[10.0, 20.0]}}
      ]
    })";
    auto g = GeoJsonParser::parse(text);
    EXPECT_EQ(g.num_nodes(), 1u);
    EXPECT_DOUBLE_EQ(g.node(0).coord.lon, 10.0);
    EXPECT_DOUBLE_EQ(g.node(0).coord.lat, 20.0);
}

TEST(GeoJsonParser, ParsesLineStringIntoEdges) {
    const std::string text = R"({
      "type":"FeatureCollection",
      "features":[
        {"type":"Feature","properties":{},
         "geometry":{"type":"LineString","coordinates":[[0,0],[1,0],[2,0]]}}
      ]
    })";
    auto g = GeoJsonParser::parse(text);
    EXPECT_EQ(g.num_nodes(), 3u);
    // 2 LineString segments, undirected => 4 directed edges
    EXPECT_EQ(g.num_edges(), 4u);
}

TEST(GeoJsonParser, SnapsSharedEndpoints) {
    // Two LineStrings share the (1,0) coordinate; should snap to single node.
    const std::string text = R"({
      "type":"FeatureCollection",
      "features":[
        {"type":"Feature","properties":{},
         "geometry":{"type":"LineString","coordinates":[[0,0],[1,0]]}},
        {"type":"Feature","properties":{},
         "geometry":{"type":"LineString","coordinates":[[1,0],[2,0]]}}
      ]
    })";
    auto g = GeoJsonParser::parse(text);
    EXPECT_EQ(g.num_nodes(), 3u);
}

TEST(GeoJsonParser, EdgeWeightsAreHaversineMeters) {
    const std::string text = R"({
      "type":"FeatureCollection",
      "features":[
        {"type":"Feature","properties":{},
         "geometry":{"type":"LineString","coordinates":[[-0.1276,51.5074],[2.3522,48.8566]]}}
      ]
    })";
    auto g = GeoJsonParser::parse(text);
    ASSERT_EQ(g.num_nodes(), 2u);
    ASSERT_FALSE(g.neighbors(0).empty());
    EXPECT_NEAR(g.neighbors(0)[0].weight, 343'500.0, 5'000.0);
}
