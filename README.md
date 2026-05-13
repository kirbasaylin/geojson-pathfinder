# GeoJSON Pathfinding Engine

[![CI](https://github.com/aylinkirbas/geojson-pathfinder/actions/workflows/ci.yml/badge.svg)](https://github.com/aylinkirbas/geojson-pathfinder/actions/workflows/ci.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![License](https://img.shields.io/badge/license-MIT-green)

A C++17 library that parses [GeoJSON](https://geojson.org/) `FeatureCollection`s into a graph and computes shortest paths with Dijkstra's algorithm backed by an indexed binary min-heap. Sub-millisecond queries on city-scale road networks.

## What it does

- **Parser** — streaming GeoJSON reader (no full DOM allocation). Two `LineString`s sharing a coordinate share a node.
- **Graph** — dense adjacency list with `NodeId` indices and contiguous storage.
- **Dijkstra** — `O((V + E) log V)` with `decrease-key` in `O(log n)` via an indexed min-heap.
- **Distance** — edge weights are great-circle distances (haversine, in meters).

## Build & Run

### Windows (Visual Studio)
1. Install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/) with the "Desktop development with C++" workload (includes CMake).
2. Open the project folder in Visual Studio — it will detect `CMakeLists.txt` automatically.
3. Click **Build → Build All**, then **Select Startup Item → gjp_cli.exe** and run.

Or from the **Developer Command Prompt for VS 2022**:
```cmd
cmake -S . -B build
cmake --build build --config Release
build\Release\gjp_cli.exe data\sample.geojson 0 4
```

### macOS / Linux
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/gjp_cli data/sample.geojson 0 4
```

Expected output:
```
Loaded graph: 6 nodes, 12 edges in 411 us
Query time: 1 us
Distance: 330.8 m
Path (3 nodes): 0 1 4
```

Node IDs are assigned in the order unique coordinates first appear in the input.

## Use as a library

```cpp
#include "geojson_pathfinder/geojson_parser.hpp"
#include "geojson_pathfinder/dijkstra.hpp"

auto graph  = gjp::GeoJsonParser::parse_file("roads.geojson");
auto result = gjp::Dijkstra(graph).shortest_path(/*source=*/0, /*target=*/42);

if (result.found) {
    std::cout << "Distance: " << result.total_distance << " m\n";
    for (auto id : result.path) std::cout << id << " ";
}
```

## Run the tests
```bash
cd build && ctest --output-on-failure
```

Coverage spans the indexed min-heap, the graph, Dijkstra (triangle shortcut, disconnected nodes, source==target, grids, full distance vector), and the parser (empty input, point, linestring, shared-endpoint snapping, haversine weights).

## Architecture

```
include/geojson_pathfinder/
  types.hpp           NodeId, Coordinate, Edge, Node
  graph.hpp           Adjacency list + haversine helper
  priority_queue.hpp  IndexedMinHeap (decrease-key in O(log n))
  dijkstra.hpp        Dijkstra solver
  geojson_parser.hpp  Streaming GeoJSON -> Graph

src/                  Implementations
tests/                GoogleTest unit tests (fetched by CMake)
data/                 Sample GeoJSON inputs
```

## Complexity

| Operation | Time | Space |
|---|---|---|
| `IndexedMinHeap::push` / `decrease_key` / `pop_min` | O(log V) | O(V) |
| `Dijkstra::shortest_path` | O((V + E) log V) | O(V) |
| `GeoJsonParser::parse` | O(N + C) where N = features, C = coords | O(V + E) |

## Author

**Aylin Kirbas** — ECE & GIS @ The Ohio State University. [aylinnkirbas_@outlook.com](mailto:aylinnkirbas_@outlook.com) · [LinkedIn](https://www.linkedin.com/in/aylin-kirbas-200139267)

## License

MIT — see [LICENSE](LICENSE).
