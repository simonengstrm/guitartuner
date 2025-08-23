cmake_minimum_required(VERSION 3.24)
set(CMAKE_GENERATOR "Unix Makefiles")
project(PitchDetector)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER "clang++")

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

find_package(PortAudio REQUIRED)

add_executable(PitchDetector
    main.cpp
    audio_engine.cpp
    freq_analysis.cpp
)

target_link_libraries(PitchDetector PRIVATE
    portaudio
)

if(WIN32)
    target_link_libraries(PitchDetector PRIVATE winmm)
endif()
