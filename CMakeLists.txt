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

# ---- Other dependencies ----

## Import dlib with FetchContent, as CUDA support cannot be conditionally enabled via vcpkg manifest mode
#if (DLIB_BUILD)
#    message("Fetching and building dlib")
#    include(FetchContent)
#    FetchContent_Declare(dlib
#            GIT_REPOSITORY https://github.com/davisking/dlib.git
#            GIT_TAG v19.24.2
#    )
#    FetchContent_MakeAvailable(dlib)
#else (DLIB_BUILD)
#    find_package(dlib CONFIG REQUIRED)
#endif (DLIB_BUILD)

# ---- Declare library ----

add_library(
        AlbumArchitect_lib OBJECT
        source/album/photo.cpp
        source/album/photo_dnn.cpp
        source/album/photo.h
        source/album/album_exception.h
        source/album/face_classifier.cpp
        source/album/face_classifier.h
        source/album/text_classifier.cpp
        source/album/text_classifier.h
        source/album/util.cpp
        source/album/util.h
        source/album/album.cpp
        source/album/album.h
        source/config/config.cpp
        source/config/config.h
        source/support/serialize/cvmat.h
)

target_include_directories(
        AlbumArchitect_lib ${warning_guard}
        PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(AlbumArchitect_lib PUBLIC cxx_std_20)

find_package(Boost REQUIRED COMPONENTS thread filesystem)

find_package(OpenCV 4.8.0 CONFIG REQUIRED)
find_package(lcms2 CONFIG REQUIRED) # Required for OpenImageIO
find_package(OpenEXR CONFIG REQUIRED) # Required for OpenImageIO
find_package(OpenImageIO CONFIG REQUIRED) # Load any type of image format
find_package(glog CONFIG REQUIRED) # Logging support
find_package(Tesseract CONFIG REQUIRED) # OCR support
find_package(cereal CONFIG REQUIRED) # Serialization library
find_package(yaml-cpp CONFIG REQUIRED)

find_package(dlib CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

target_link_libraries(AlbumArchitect_lib PRIVATE
        Boost::thread
        Boost::filesystem
        Boost::headers
        OpenEXR::OpenEXR
        dlib::dlib
        Tesseract::libtesseract
        yaml-cpp::yaml-cpp)
target_link_libraries(AlbumArchitect_lib PUBLIC
        fmt::fmt
        OpenImageIO::OpenImageIO
        glog::glog
        opencv_core
        cereal::cereal
        range-v3::range-v3)

add_custom_target(AlbumArchitect_data)
add_dependencies(AlbumArchitect_lib AlbumArchitect_data)

# ---- Declare executable ----

add_definitions(-DDLIB_NO_GUI_SUPPORT)

add_executable(AlbumArchitect_exe source/main.cpp)
add_executable(AlbumArchitect::exe ALIAS AlbumArchitect_exe)

set_property(TARGET AlbumArchitect_exe PROPERTY OUTPUT_NAME AlbumArchitect)

target_compile_features(AlbumArchitect_exe PRIVATE cxx_std_20)

target_link_libraries(AlbumArchitect_exe PRIVATE AlbumArchitect_lib opencv_highgui)

# Copy runtime data
set(RUNTIME_MODELS ${CMAKE_SOURCE_DIR}/data)
#set(OPENCV_MODELS ${CMAKE_SOURCE_DIR}/3rdparty/opencv_zoo/models)
#list(APPEND OPENCV_MODEL_FILES
#        ${OPENCV_MODELS}/face_detection_yunet/face_detection_yunet_2023mar.onnx
#        ${OPENCV_MODELS}/face_detection_yunet/face_detection_yunet_2023mar_int8.onnx
#)

#add_custom_command(TARGET AlbumArchitect_data
#        COMMAND ${CMAKE_COMMAND} -E make_directory
#        ${RUNTIME_MODELS}
#        COMMAND ${CMAKE_COMMAND} -E copy
#        ${OPENCV_MODEL_FILES} ${RUNTIME_MODELS})

# .. dlib
if (DLIB_DOWNLOAD_DATA)
    message("Downloading DLIB models")
    add_custom_command(TARGET AlbumArchitect_data
            COMMAND ${CMAKE_COMMAND} -E make_directory
            ${RUNTIME_MODELS})
    find_program(7ZIP "7z")
    if (7ZIP)
        file(DOWNLOAD dlib.net/files/mmod_human_face_detector.dat.bz2 ${RUNTIME_MODELS}/mmod_human_face_detector.dat.bz2)
        execute_process(
                COMMAND 7z x ${RUNTIME_MODELS}/mmod_human_face_detector.dat.bz2 -aos -o${RUNTIME_MODELS})
    else (7ZIP)
        message(FATAL_ERROR "Couldn't find 7zip for unzipping DLIB models")
    endif (7ZIP)
endif (DLIB_DOWNLOAD_DATA)

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
