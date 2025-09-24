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

# Handle Windows icon resource
if(WIN32)
    set(APP_ICON "${ASSETS_DIR}/images/app_icon.ico")
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

# GUI app flags - KEEP THE AUTOUIC_SEARCH_PATHS!
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOUIC_SEARCH_PATHS "${FORMS_DIR}"
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# Copy sound files to executable directory
file(GLOB_RECURSE SOUNDS "${ASSETS_DIR}/sounds/*.wav")
foreach(SOUND ${SOUNDS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SOUND}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

# ... rest of your CMakeLists.txt (tests) remains unchanged
