cmake_minimum_required(VERSION 3.24)
set(CMAKE_GENERATOR "Unix Makefiles")
project(PitchDetector)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

find_package(PortAudio REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(src)

target_link_libraries(PitchDetector PRIVATE
    portaudio
    OpenGL::GL
)

target_include_directories(PitchDetector PRIVATE
    ${PORTAUDIO_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
)

set_target_properties(PitchDetector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(WIN32)
    target_link_libraries(PitchDetector PRIVATE winmm)
endif()
