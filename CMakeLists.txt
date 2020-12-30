cmake_minimum_required(VERSION 3.10)

# set the project name
project(DecodeAudio)

# set the project name and version
project(DecodeAudio VERSION 1.0)

set(LINK_PATH ${CMAKE_SOURCE_DIR}/../../3rd/FFmpeg/lib)
set(FFmpegInclude ${CMAKE_SOURCE_DIR}/../../3rd/FFmpeg/include)

include_directories(${FFmpegInclude})
link_directories(${LINK_PATH})

# add the executable
add_executable(DecodeAudio
    decoder.cpp
    test.cpp)

set(mFFmpeg avcodec avformat swscale avutil avfilter swresample)
target_link_libraries(DecodeAudio ${mFFmpeg})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

configure_file(DecodeAudioConfig.h.in DecodeAudioConfig.h)

target_include_directories(DecodeAudio PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
