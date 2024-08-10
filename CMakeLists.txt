cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
        AlbumArchitect
        VERSION 0.1.0
        DESCRIPTION "Project for managing home photos."
        HOMEPAGE_URL "https://albumarchitect.mares.casa"
        LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# Remove extra targets from CTest
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

# ---- Declare library ----

add_library(
        AlbumArchitect_lib OBJECT
        source/files/tree.cpp
        source/files/tree.h
        source/files/graph.cpp
        source/files/graph.h
        source/files/helper.cpp
        source/files/helper.h
        source/album/image.cpp
        source/album/image.h
)

target_include_directories(
        AlbumArchitect_lib ${warning_guard}
        PUBLIC
        "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(AlbumArchitect_lib PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost 1.83 REQUIRED COMPONENTS graph serialization filesystem)
find_package(absl CONFIG REQUIRED)
find_package(OpenCV 4.8 REQUIRED COMPONENTS opencv_img_hash)
find_package(OpenImageIO 2.5 REQUIRED CONFIG)
target_link_libraries(AlbumArchitect_lib PRIVATE
        fmt::fmt
        spdlog::spdlog
        Boost::headers
        absl::btree
        absl::hash)
target_link_libraries(AlbumArchitect_lib PUBLIC
        Boost::graph
        Boost::filesystem
        Boost::serialization
        opencv_core
        opencv_imgproc
        opencv_imgcodecs
        opencv_img_hash
        OpenImageIO::OpenImageIO)

# ---- Declare executable ----

add_executable(AlbumArchitect_exe source/main.cpp)
add_executable(AlbumArchitect::exe ALIAS AlbumArchitect_exe)

set_property(TARGET AlbumArchitect_exe PROPERTY OUTPUT_NAME AlbumArchitect)

target_compile_features(AlbumArchitect_exe PRIVATE cxx_std_20)

target_link_libraries(AlbumArchitect_exe PRIVATE AlbumArchitect_lib fmt::fmt spdlog::spdlog)

# ---- Install rules ----

if (NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif ()

# ---- Developer mode ----

if (NOT AlbumArchitect_DEVELOPER_MODE)
    return()
elseif (NOT PROJECT_IS_TOP_LEVEL)
    message(
            AUTHOR_WARNING
            "Developer mode is intended for developers of AlbumArchitect"
    )
endif ()

include(cmake/dev-mode.cmake)
