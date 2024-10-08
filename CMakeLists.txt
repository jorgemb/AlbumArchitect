cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

# Add required manifest features
if(USE_GPU EQUAL "ON")
    list(APPEND VCPKG_MANIFEST_FEATURES "gpu")
endif ()


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
        source/album/hash.cpp
        source/album/hash.h
        source/helper/boost_serialization_cvmat.h
        source/files/common.h
        source/helper/cv_mat_operations.h
        source/album/photo.cpp
        source/album/photo.h
        source/album/photo_metadata.cpp
        source/album/photo_metadata.h
)

target_include_directories(
        AlbumArchitect_lib ${warning_guard}
        PUBLIC
        "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(AlbumArchitect_lib PUBLIC cxx_std_17)

# Add support for TBB on linux
if (UNIX)
    # According to fix here: https://github.com/microsoft/vcpkg/issues/31617
    add_compile_definitions(TBB_USE_ASSERT)
endif (UNIX)

find_package(fmt REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost 1.83 REQUIRED COMPONENTS graph serialization filesystem)
find_package(absl CONFIG REQUIRED)
find_package(OpenCV 4.8 REQUIRED COMPONENTS opencv_img_hash)
find_package(OpenImageIO 2.5 REQUIRED CONFIG)
find_package(unofficial-hash-library CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
target_link_libraries(AlbumArchitect_lib PRIVATE
        fmt::fmt
        spdlog::spdlog
        Boost::headers
        absl::btree
        absl::hash
        unofficial::hash-library
        magic_enum::magic_enum
        TBB::tbb)
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
