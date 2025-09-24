cmake_minimum_required(VERSION 3.16)

project(MindLeap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automatic features
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 modules
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Multimedia)

# Source directories
set(FORMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

# Main executable
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.h" "${INCLUDE_DIR}/*.hpp")
file(GLOB_RECURSE UI "${FORMS_DIR}/*.ui")
set(APP_ICON "${ASSETS_DIR}/images/app_icon.ico")
set(APP_RC "${ASSETS_DIR}/rc/app_icon.rc")
configure_file(${APP_ICON} ${CMAKE_BINARY_DIR}/images/app_icon.ico COPYONLY)

add_executable(${PROJECT_NAME}
    ${SOURCES} ${HEADERS} ${UI} ${APP_RC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Multimedia
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOUIC_SEARCH_PATHS ${FORMS_DIR}
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# Copy sound files
file(GLOB_RECURSE SOUNDS "${ASSETS_DIR}/sounds/*.wav")
foreach(SOUND ${SOUNDS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SOUND}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

# Catch2 tests
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

# Test executable
file(GLOB_RECURSE BACKEND_SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE BACKEND_HEADERS "${INCLUDE_DIR}/*.hpp")
list(FILTER BACKEND_SOURCES EXCLUDE REGEX ".*/Frontend/.*\\.cpp$")
list(FILTER BACKEND_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
list(FILTER BACKEND_HEADERS EXCLUDE REGEX ".*/Frontend/.*\\.hpp$")
file(GLOB_RECURSE TEST_SOURCES "${TESTS_DIR}/*.cpp")

add_executable(${PROJECT_NAME}_tests
    ${BACKEND_SOURCES} ${BACKEND_HEADERS} ${TEST_SOURCES}
)

target_include_directories(${PROJECT_NAME}_tests PRIVATE ${INCLUDE_DIR})
set_target_properties(${PROJECT_NAME}_tests PROPERTIES
    AUTOUIC OFF
    AUTOMOC OFF
    AUTORCC OFF
)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Sql
)

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME}_tests)

# Deployment per platform
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt PATHS $ENV{QTDIR}/bin NO_DEFAULT_PATH)
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Running windeployqt for Windows"
        )
    endif()
elseif(APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt PATHS $ENV{QTDIR}/bin /usr/local/opt/qt/bin NO_DEFAULT_PATH)
    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${MACDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Running macdeployqt for macOS"
        )
    endif()
endif()
# Linux will run in-place; Qt libs must be in LD_LIBRARY_PATH
