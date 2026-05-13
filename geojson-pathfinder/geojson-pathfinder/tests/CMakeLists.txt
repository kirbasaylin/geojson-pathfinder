include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(gjp_tests
    test_graph.cpp
    test_priority_queue.cpp
    test_dijkstra.cpp
    test_geojson_parser.cpp
)
target_link_libraries(gjp_tests PRIVATE gjp_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(gjp_tests)
