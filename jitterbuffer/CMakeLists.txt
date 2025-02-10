cmake_minimum_required(VERSION 3.10)

# Set the project name and specify C as the language
project(JitterBufferTest LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_CONFIG_H")

# Specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Set the default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Add the executable
add_executable(JitterBufferTest
    main.c
    jitter.c
)

# Include directories
target_include_directories(JitterBufferTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries if needed (e.g., if jitter.c depends on any libraries)
# target_link_libraries(JitterBufferTest <library_name>)