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

# Handle Windows icon resource
if(WIN32)
    set(APP_RC "${ASSETS_DIR}/rc/app_icon.rc")
    configure_file(${APP_ICON} ${CMAKE_BINARY_DIR}/images/app_icon.ico COPYONLY)
else()
    set(APP_RC "")
endif()

add_executable(${PROJECT_NAME}
    ${SOURCES} ${HEADERS} ${UI} ${APP_RC}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Multimedia
)

# GUI app flags
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOUIC_SEARCH_PATHS ${FORMS_DIR}
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# macOS specific bundle configuration
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos/Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME "MindLeap"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2024 MindLeap. All rights reserved."
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.MindLeap"
    )
    
    # Install the app bundle to the build directory
    install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)
endif()

# Copy sound files (only for non-bundle builds or as a fallback)
file(GLOB_RECURSE SOUNDS "${ASSETS_DIR}/sounds/*.wav")
if(NOT APPLE)  # For macOS, we'll handle this in the bundle
    foreach(SOUND ${SOUNDS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SOUND}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endforeach()
endif()

# Platform-specific deployment (run after build is complete)
if(WIN32)
    # Windows deployment - use a custom target that runs after build
    add_custom_target(deploy_win ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
        COMMAND windeployqt $<TARGET_FILE:${PROJECT_NAME}>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${PROJECT_NAME}
        COMMENT "Deploying Windows application"
    )
elseif(APPLE)
    # macOS deployment - separate target that depends on the main build
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt)
    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_target(deploy_mac ALL
            COMMAND ${CMAKE_COMMAND} -E echo "Running macdeployqt..."
            COMMAND ${MACDEPLOYQT_EXECUTABLE} MindLeap.app -verbose=3
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${PROJECT_NAME}
            COMMENT "Deploying macOS application"
        )
        
        # Copy sound files into the app bundle
        foreach(SOUND ${SOUNDS})
            get_filename_component(SOUND_NAME ${SOUND} NAME)
            add_custom_command(TARGET deploy_mac POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SOUND}
                MindLeap.app/Contents/MacOS/
                COMMENT "Copying sound file: ${SOUND_NAME}"
            )
        endforeach()
    endif()
endif()

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
