cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the path to your Qt6 installation
set(CMAKE_PREFIX_PATH "D:/Qt/6.8.1/mingw_64/lib/cmake")

# Enable Qt's automatic features
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

# Specify the .ui files
set(UI_FILES "${FORMS_DIR}/mainwindow.ui"
        include/Backend/Database/queries.hpp)  # Path to the .ui file

# Source and header files
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp" "${INCLUDE_DIR}/*.h")

# Add the executable
add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${UI_FILES}  # Add the .ui files here
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