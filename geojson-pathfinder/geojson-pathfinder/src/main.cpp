#include "geojson_pathfinder/dijkstra.hpp"
#include "geojson_pathfinder/geojson_parser.hpp"
#include "geojson_pathfinder/graph.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void print_usage() {
    std::cerr <<
        "Usage:\n"
        "  gjp_cli <geojson_file> <source_id> <target_id>\n"
        "\n"
        "  source_id / target_id are 0-based node indices, assigned in the order\n"
        "  unique coordinates appear in the input GeoJSON.\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 4) {
        print_usage();
        return EXIT_FAILURE;
    }
    const std::string path = argv[1];
    const auto source = static_cast<gjp::NodeId>(std::stoul(argv[2]));
    const auto target = static_cast<gjp::NodeId>(std::stoul(argv[3]));

    try {
        const auto t0 = std::chrono::steady_clock::now();
        auto graph = gjp::GeoJsonParser::parse_file(path);
        const auto t1 = std::chrono::steady_clock::now();

        std::cout << "Loaded graph: " << graph.num_nodes() << " nodes, "
                  << graph.num_edges() << " edges in "
                  << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()
                  << " us\n";

        gjp::Dijkstra solver(graph);
        const auto t2 = std::chrono::steady_clock::now();
        const auto result = solver.shortest_path(source, target);
        const auto t3 = std::chrono::steady_clock::now();

        std::cout << "Query time: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()
                  << " us\n";

        if (!result.found) {
            std::cout << "No path from " << source << " to " << target << "\n";
            return EXIT_SUCCESS;
        }
        std::cout << "Distance: " << result.total_distance << " m\n";
        std::cout << "Path (" << result.path.size() << " nodes): ";
        for (auto id : result.path) std::cout << id << " ";
        std::cout << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
