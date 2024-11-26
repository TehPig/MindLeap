# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MyProject)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)

# Add all .cpp files from src/ and subdirectories
file(GLOB_RECURSE SOURCES src/**/*.cpp)

# Include the 'include' directory for header files
include_directories(include)

# Add the executable
add_executable(anki ${SOURCES})