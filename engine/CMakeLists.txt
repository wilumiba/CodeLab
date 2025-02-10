# 指定 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称和语言
project(engine LANGUAGES C CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 定义源文件和头文件
set(SOURCES
    main.cpp
    AudioData/AudioData.cpp
    Player/CoreAudioPlayer.cpp
    HandlerChain/AudioHandlerChain.cpp
    Accelerator/Accelerator.cpp
    Accelerator/sonic.c
    SoundToucher/SoundToucher.cpp
    Codec/OpusEncoder.cpp
    Codec/OpusDecoder.cpp
    AudioHelper/AudioHelper.cpp
)

set(HEADERS
    IAudioData.h
    AudioData/AudioData.h
    Player/CoreAudioPlayer.h
    HandlerChain/AudioHandlerChain.h
    Accelerator/Accelerator.h
    Accelerator/sonic.h
    SoundToucher/SoundToucher.h
    Codec/OpusEncoder.h
    Codec/OpusDecoder.h
)

# 定义一个可执行文件目标
add_executable(${PROJECT_NAME} ${SOURCES})

# 包含头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE 
                           ${CMAKE_CURRENT_SOURCE_DIR} 
                           AudioData 
                           Player
                           HandlerChain
                           Accelerator
                           SoundToucher
                           Codec
                           AudioHelper
                           ../wsola/soundtouch/include
                           ../opus/include)


# Specify the path to the SoundTouch library
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Find the SoundTouch library
find_library(SOUNDTOUCH_LIB SoundTouch PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Find the Opus library
find_library(OPUS_LIB opus PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libs)

message("CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")
message(STATUS "SoundTouch library: ${SOUNDTOUCH_LIB}")
message(STATUS "Opus library: ${OPUS_LIB}")
# Check if the libraries were found
if(NOT SOUNDTOUCH_LIB)
    message(FATAL_ERROR "SoundTouch library not found")
endif()

if(NOT OPUS_LIB)
    message(FATAL_ERROR "Opus library not found")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreAudio" "-framework AudioToolbox" ${SOUNDTOUCH_LIB} ${OPUS_LIB})