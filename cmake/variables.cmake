# ---- Developer mode ----

# Developer mode enables targets and code paths in the CMake scripts that are
# only relevant for the developer(s) of AlbumArchitect
# Targets necessary to build the project must be provided unconditionally, so
# consumers can trivially build and package the project
if(PROJECT_IS_TOP_LEVEL)
  option(AlbumArchitect_DEVELOPER_MODE "Enable developer mode" OFF)
endif()

# ---- Warning guard ----

# target_include_directories with the SYSTEM modifier will request the compiler
# to omit warnings from the provided paths, if the compiler supports that
# This is to provide a user experience similar to find_package when
# add_subdirectory or FetchContent is used to consume this project
set(warning_guard "")
if(NOT PROJECT_IS_TOP_LEVEL)
  option(
      AlbumArchitect_INCLUDES_WITH_SYSTEM
      "Use SYSTEM modifier for AlbumArchitect's includes, disabling warnings"
      ON
  )
  mark_as_advanced(AlbumArchitect_INCLUDES_WITH_SYSTEM)
  if(AlbumArchitect_INCLUDES_WITH_SYSTEM)
    set(warning_guard SYSTEM)
  endif()
endif()

# ---- Additional configuration ----

# Whether to use the local dlib with find_package or use Fetch to include
set(DLIB_BUILD ON CACHE BOOL "Fetch and compile dlib")
set(DLIB_DOWNLOAD_DATA ON CACHE BOOL "Download model data for dlib")

