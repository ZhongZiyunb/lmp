cmake_minimum_required(VERSION 3.4) # WINDOWS_EXPORT_ALL_SYMBOLS needs version >= 3.4
project(lmp)
SET(CMAKE_BUILD_TYPE "Debug")
option(RAZOR_BUILD_SHARED "Build razor as shared library" OFF)
option(RAZOR_BUILD_SIM_TRANSPORT "Build sim_transport" ON)

file(GLOB_RECURSE CODEC_SRC_FILES ./codec/*)

# add_library(libcodec ${CODEC_SRC_FILES})


add_library(libecho EchoManager.cpp EchoManager.hpp LMPLogger.cpp LMPLogger.hpp 
                VideoPlayer.hpp VideoPlayer.cpp VideoReader.hpp VideoReader.cpp
                VideoPlayThread.hpp VideoPlayThread.cpp VideoReadThread.hpp VideoReadThread.cpp
                BaseThread.cpp BaseThread.hpp ${CODEC_SRC_FILES})

target_include_directories(libecho PUBLIC
    ../sim_transport
    ../sim_transport/fec
    
)

# set(FFMPEG_PATH "/usr/local/ffmpeg/lib")
target_link_libraries(libecho PUBLIC
    $<TARGET_NAME:sim_transport>

    avcodec
    avutil
    avformat
    swscale
    # ${FFMPEG_PATH}/libavcodec.so 
    # ${FFMPEG_PATH}/libavutil.so
    # ${FFMPEG_PATH}/libavformat.so
    # ${FFMPEG_PATH}/libswscale.so
)

# include_directories("/usr/local/ffmpeg/include/")
# include_directories("./codec/ffmpeg/include/")

include_directories("./codec/")
include_directories("./codec/x264/")
include_directories("./codec/x265/")
include_directories("../common/")
include_directories("../cc/")
include_directories("../bbr/")
include_directories("../estimator/")
include_directories("../pacing/")
include_directories("../remb/")


file(GLOB_RECURSE RAZOR_SRC_FILES 
    ../cc/*
    ../bbr/*
    ../common/*
    ../estimator/*
    ../pacing/*
    ../remb/*
)

if (WIN32)
    list(FILTER RAZOR_SRC_FILES EXCLUDE REGEX "posix/posix.c")
else()
    list(FILTER RAZOR_SRC_FILES EXCLUDE REGEX "windows/mscc.c")
endif()

if (RAZOR_BUILD_SHARED)
    add_library(razor STATIC ${RAZOR_SRC_FILES})
    target_link_libraries(razor PUBLIC
        pthread
    )
else ()
    add_library(razor SHARED ${RAZOR_SRC_FILES})
    target_link_libraries(razor PUBLIC
        pthread
    )
endif ()

if (RAZOR_BUILD_SIM_TRANSPORT)
    file(GLOB_RECURSE SIM_TRANSPORT 
        ../sim_transport/*
    )
    add_library(sim_transport ${SIM_TRANSPORT})
    target_include_directories(sim_transport PUBLIC
        ../sim_transport
        ../sim_transport/fec
    )
    target_link_libraries(sim_transport PUBLIC
        $<TARGET_NAME:razor>
    )
    set_target_properties(sim_transport
    PROPERTIES
        C_STANDARD 99
    )
endif()

set(LMP_SRC_FILES main.cpp)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} libecho razor sim_transport)

# target_link_libraries(${PROJECT_NAME} fmt)
