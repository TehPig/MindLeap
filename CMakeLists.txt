cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the path to your Qt6 installation
set(CMAKE_PREFIX_PATH "D:/Qt/6.8.1/mingw_64/lib/cmake")

# Enable Qt's automatic features for the main executable
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

project(MindLeap)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# Paths for source files and forms
set(FORMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/forms")    # Directory for .ui files
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include") # Header files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")         # Source files
set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")     # Test files
set(LIBRARIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Backend/Libraries") # Libraries directory
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets") # Assets files

set(APP_ICON "${ASSETS_DIR}/images/app_icon.ico")
set(APP_RC "${ASSETS_DIR}/rc/app_icon.rc")
configure_file(${APP_ICON} ${CMAKE_BINARY_DIR}/images/app_icon.ico COPYONLY)

# Source and header files
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp" "${INCLUDE_DIR}/*.h")
file(GLOB_RECURSE UI "${FORMS_DIR}/*.ui")

# Add the main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI} ${APP_RC})

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
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)

# Enable compiler optimizations and link-time optimization (LTO)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")

# Add Catch2 for tests
include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.0 # Replace with the desired version
)
FetchContent_MakeAvailable(Catch2)

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
        ${INCLUDE_DIR})

# Disable AUTOUIC, AUTOMOC, AUTORCC for the tests
set_target_properties(${PROJECT_NAME}_tests PROPERTIES
        WIN32_EXECUTABLE OFF
        AUTOUIC OFF
        AUTOMOC OFF
        AUTORCC OFF
)

# Link Catch2 with the test executable
target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        Catch2::Catch2WithMain # Links Catch2 and provides the main function
        Qt6::Core
        Qt6::Sql
)

# Copy required Qt DLLs to the build directory
add_custom_command(TARGET ${PROJECT_NAME}_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt6_DIR}/../../../bin/Qt6Core.dll"
        "${Qt6_DIR}/../../../bin/Qt6Sql.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}_tests>
)

# Enable CTest
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME}_tests)