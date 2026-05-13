cmake_minimum_required(VERSION 3.16)
project(geojson_pathfinder
    VERSION 1.0.0
    DESCRIPTION "GeoJSON Pathfinding Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# Library target
add_library(gjp_lib
    src/geojson_parser.cpp
    src/graph.cpp
    src/dijkstra.cpp
    src/priority_queue.cpp
)
target_include_directories(gjp_lib PUBLIC include)

# CLI executable
add_executable(gjp_cli src/main.cpp)
target_link_libraries(gjp_cli PRIVATE gjp_lib)

# Tests
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS gjp_cli RUNTIME DESTINATION bin)
install(TARGETS gjp_lib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
