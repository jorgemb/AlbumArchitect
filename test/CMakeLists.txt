# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(AlbumArchitectTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(Catch)

# ---- Tests ----

add_executable(AlbumArchitect_test source/AlbumArchitect_test.cpp
        source/album/photo_test.cpp
        source/config/config_test.cpp
        source/support_test.cpp)
target_link_libraries(
        AlbumArchitect_test PRIVATE
        AlbumArchitect_lib
        Catch2::Catch2
        OpenImageIO::OpenImageIO
        opencv_core
        Boost::headers
)
target_compile_features(AlbumArchitect_test PRIVATE cxx_std_20)

catch_discover_tests(AlbumArchitect_test
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Add required files for testing
add_custom_command(TARGET AlbumArchitect_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data/ ${CMAKE_CURRENT_SOURCE_DIR}/data/)

# ---- End-of-file commands ----

add_folders(Test)
