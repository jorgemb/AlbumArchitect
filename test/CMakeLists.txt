# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(AlbumArchitectTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----
set(TEST_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

add_executable(AlbumArchitect_test source/AlbumArchitect_test.cpp
        source/files.cpp)
target_link_libraries(
        AlbumArchitect_test PRIVATE
        AlbumArchitect_lib
        Catch2::Catch2WithMain
        Boost::graph
)
target_compile_features(AlbumArchitect_test PRIVATE cxx_std_17)
target_compile_definitions(AlbumArchitect_test PRIVATE TEST_RESOURCES_DIR="${TEST_RESOURCES_DIR}")

catch_discover_tests(AlbumArchitect_test)

# ---- End-of-file commands ----

add_folders(Test)
