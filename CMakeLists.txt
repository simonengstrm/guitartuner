cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_COMPILER "clang++")
project(PitchDetector)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g -Wall -Wextra -Wpedantic")

add_subdirectory(src)

find_package(portaudio REQUIRED HINTS "/extern/portaudio/include" LIBRARY "/extern/portaudio/lib/libportaudio.la")

target_link_libraries(PitchDetector PRIVATE
    portaudio
)

target_include_directories(PitchDetector PRIVATE
    "extern/portaudio/src/common" # Needed for ringbuffer util
)

set_target_properties(PitchDetector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(WIN32)
    target_link_libraries(PitchDetector PRIVATE winmm)
endif()
