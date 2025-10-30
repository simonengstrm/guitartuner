# Add all files in src/ to the build
file(GLOB SRC_FILES "*.cpp" "*.h")
add_executable(PitchDetector ${SRC_FILES})