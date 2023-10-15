cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    AlbumArchitect
    VERSION 0.1.0
    DESCRIPTION "AlbumArchitect is your go-to solution for streamlined photo management. With cutting-edge face detection capabilities, it automatically recognizes and labels individuals in your photos. It also swiftly identifies similar or duplicate images, simplifying the organization of your collection. Effortlessly construct meaningful albums and elegantly sort and structure your treasured memories with AlbumArchitect – your photo organization's best friend."
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    AlbumArchitect_lib OBJECT
    source/lib.cpp
        source/album/photo.cpp
        source/album/photo.h
        source/album/album_exception.h
)

target_include_directories(
    AlbumArchitect_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(AlbumArchitect_lib PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread filesystem)

find_package(TBB REQUIRED)
find_package(lcms2 CONFIG REQUIRED) # Required for OpenImageIO
find_package(OpenCV 4 CONFIG REQUIRED)

find_package(OpenImageIO CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)

target_link_libraries(AlbumArchitect_lib PRIVATE fmt::fmt Boost::filesystem Boost::thread Boost::headers OpenEXR::OpenEXR TBB::tbb)
target_link_libraries(AlbumArchitect_lib PUBLIC OpenImageIO::OpenImageIO glog::glog opencv_core)

# ---- Declare executable ----

add_executable(AlbumArchitect_exe source/main.cpp)
add_executable(AlbumArchitect::exe ALIAS AlbumArchitect_exe)

set_property(TARGET AlbumArchitect_exe PROPERTY OUTPUT_NAME AlbumArchitect)

target_compile_features(AlbumArchitect_exe PRIVATE cxx_std_20)

target_link_libraries(AlbumArchitect_exe PRIVATE AlbumArchitect_lib opencv_highgui)

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
