cmake_minimum_required(VERSION 3.24)
set(CMAKE_GENERATOR "Unix Makefiles")
project(PitchDetector)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PortAudio REQUIRED)

add_executable(PitchDetector main.cpp freq_analysis.h freq_analysis.cpp)

target_include_directories(PitchDetector PRIVATE
    ${portaudio_SOURCE_DIR}/include
)

target_link_libraries(PitchDetector PRIVATE
    portaudio
)

if(WIN32)
    target_link_libraries(PitchDetector PRIVATE winmm)
endif()
