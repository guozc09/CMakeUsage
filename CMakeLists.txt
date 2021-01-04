cmake_minimum_required(VERSION 3.10)

# set the project name
project(DecodeAudio)

# set the project name and version
project(DecodeAudio VERSION 1.0)

message(STATUS "***********************************************************")
message(STATUS "CMAKE_SOURCE_DIR->" ${CMAKE_SOURCE_DIR})
message(STATUS "PROJECT_SOURCE_DIR->" ${PROJECT_SOURCE_DIR})
message(STATUS "***********************************************************")

# predefine macro
add_definitions(-D __STDC_CONSTANT_MACROS)


set(LINK_PATH ${CMAKE_SOURCE_DIR}/../../3rd/FFmpeg/lib)
set(FFmpegInclude ${CMAKE_SOURCE_DIR}/../../3rd/FFmpeg/include)

include_directories(${FFmpegInclude})
link_directories(${LINK_PATH})

# add the executable
add_executable(DecodeAudio
    AudioDecoder.cpp
    test.cpp)

set(mFFmpeg avcodec avformat avutil swresample)
target_link_libraries(DecodeAudio ${mFFmpeg})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

configure_file(DecodeAudioConfig.h.in DecodeAudioConfig.h)

target_include_directories(DecodeAudio PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加对gdb的支持
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
