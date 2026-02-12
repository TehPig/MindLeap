cmake_minimum_required(VERSION 3.16)
project(MindLeap VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DPROJECT_CREATOR="TehPig")

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
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.c")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.h" "${INCLUDE_DIR}/*.hpp")
file(GLOB_RECURSE UI "${FORMS_DIR}/*.ui")
set(RESOURCES "resources.qrc")

# Discord RPC Configuration
option(USE_DISCORD "Enable Discord Rich Presence" ON)

# Search for and REMOVE all RPC files from the build by default
file(GLOB_RECURSE RPC_INTERNAL_SOURCES "${SRC_DIR}/Backend/RPC/*.cpp" "${SRC_DIR}/Backend/RPC/*.c")

if(USE_DISCORD AND WIN32)
    add_definitions(-DDISCORD_WINDOWS)
    # They are already in SOURCES from GLOB_RECURSE
else()
    # On Linux/Docker, definitively remove RPC files from sources list
    if(RPC_INTERNAL_SOURCES)
        list(REMOVE_ITEM SOURCES ${RPC_INTERNAL_SOURCES})
    endif()
    set(USE_DISCORD OFF)
endif()

# Handle Windows icon resource
if(WIN32)
    set(APP_ICON "${ASSETS_DIR}/images/app_icon.ico")
    set(APP_RC "${ASSETS_DIR}/rc/app_icon.rc")
    configure_file(${APP_ICON} ${CMAKE_BINARY_DIR}/images/app_icon.ico COPYONLY)
else()
    set(APP_RC "")
endif()

add_executable(${PROJECT_NAME}
    ${SOURCES} ${HEADERS} ${UI} ${RESOURCES} ${APP_RC}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Backend/RPC"
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(USE_DISCORD AND WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DISCORD_RPC)
endif()

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Multimedia
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32
        psapi
        advapi32
    )
endif()

# GUI app flags
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOUIC_SEARCH_PATHS "${FORMS_DIR}"
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

if(WIN32 AND MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE "-static-libgcc" "-static-libstdc++")
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

# Exclude Frontend and main from backend tests
list(FILTER BACKEND_SOURCES EXCLUDE REGEX ".*/Frontend/.*\\.cpp$")
list(FILTER BACKEND_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
list(FILTER BACKEND_SOURCES EXCLUDE REGEX ".*/Backend/RPC/.*\\.cpp$")
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

