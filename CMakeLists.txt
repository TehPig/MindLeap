cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the path to your Qt6 installation
set(CMAKE_PREFIX_PATH "D:/Qt/6.8.1/mingw_64/lib/cmake")

# Enable Qt's automatic features for the main executable
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

project(Anki)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# Paths for source files and forms
set(FORMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/forms")    # Directory for .ui files
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include") # Header files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")         # Source files
set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")     # Test files
set(LIBRARIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Backend/Libraries") # Libraries directory

# Specify the .ui files
set(UI_FILES "${FORMS_DIR}/mainwindow.ui")  # Path to the .ui file

# Source and header files
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp" "${INCLUDE_DIR}/*.h")

# Add the main executable
add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${UI_FILES} # Add the .ui files here
)

# Set AUTOUIC search paths for .ui files
set_target_properties(${PROJECT_NAME} PROPERTIES
        AUTOUIC_SEARCH_PATHS ${FORMS_DIR}  # Let AUTOUIC know where to find the .ui files
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}  # For generated ui_*.h files
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
)

# Enable compiler optimizations and link-time optimization (LTO)
#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release)
#endif()

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")

# Tests EXE
file(GLOB_RECURSE BACKEND_SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE BACKEND_HEADERS "${INCLUDE_DIR}/*.hpp")

# Exclude the Frontend directory
list(FILTER BACKEND_SOURCES EXCLUDE REGEX ".*/Frontend/.*\\.cpp$")
list(FILTER BACKEND_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
list(FILTER BACKEND_HEADERS EXCLUDE REGEX ".*/Frontend/.*\\.hpp$")

file(GLOB_RECURSE TEST_SOURCES "${TESTS_DIR}/*.cpp")

# Add the tests executable
add_executable(${PROJECT_NAME}_tests
        ${BACKEND_SOURCES}
        ${BACKEND_HEADERS}
        ${TEST_SOURCES}
)
# Include directories for test executable
target_include_directories(${PROJECT_NAME}_tests PRIVATE
        ${INCLUDE_DIR}
        ${LIBRARIES_DIR} # This ensures catch.hpp is found
)

# Disable AUTOUIC, AUTOMOC, AUTORCC for the tests
set_target_properties(${PROJECT_NAME}_tests PROPERTIES
        WIN32_EXECUTABLE OFF
        AUTOUIC OFF
        AUTOMOC OFF
        AUTORCC OFF
)

# Link Qt libraries if needed
target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        Qt6::Core
        Qt6::Sql
)
