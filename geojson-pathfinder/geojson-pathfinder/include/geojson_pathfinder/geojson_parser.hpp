#pragma once

#include "geojson_pathfinder/graph.hpp"

#include <istream>
#include <string>

namespace gjp {

// Lightweight GeoJSON parser specialized for FeatureCollections containing
// Point and LineString geometries. Points become nodes (snapped by coordinate
// equality); LineStrings become chained edges. Edge weights are great-circle
// distances in meters between consecutive coordinates.
class GeoJsonParser {
public:
    // Build a graph from a GeoJSON string.
    [[nodiscard]] static Graph parse(const std::string& geojson_text);

    // Convenience: load from a file path.
    [[nodiscard]] static Graph parse_file(const std::string& path);
};

}  // namespace gjp
