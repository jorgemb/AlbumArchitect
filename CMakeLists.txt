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

# ---- Declare library ----

add_library(
    AlbumArchitect_lib OBJECT
    source/lib.cpp
    source/files/tree.cpp
    source/files/tree.h
        source/files/graph.cpp
        source/files/graph.h
        source/files/helper.cpp
        source/files/helper.h
)

target_include_directories(
    AlbumArchitect_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(AlbumArchitect_lib PUBLIC cxx_std_17)

find_package(fmt REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost 1.84 REQUIRED COMPONENTS graph)
find_package(absl CONFIG REQUIRED)
target_link_libraries(AlbumArchitect_lib PRIVATE fmt::fmt spdlog::spdlog Boost::graph Boost::headers absl::btree absl::hash)

# ---- Declare executable ----

add_executable(AlbumArchitect_exe source/main.cpp)
add_executable(AlbumArchitect::exe ALIAS AlbumArchitect_exe)

set_property(TARGET AlbumArchitect_exe PROPERTY OUTPUT_NAME AlbumArchitect)

target_compile_features(AlbumArchitect_exe PRIVATE cxx_std_17)

target_link_libraries(AlbumArchitect_exe PRIVATE AlbumArchitect_lib fmt::fmt spdlog::spdlog)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT AlbumArchitect_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of AlbumArchitect"
  )
endif()

include(cmake/dev-mode.cmake)
