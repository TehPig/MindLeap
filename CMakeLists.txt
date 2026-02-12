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

# Select sources EXPLICITLY to avoid RPC folder on Linux by default
file(GLOB_RECURSE SOURCES 
    "${SRC_DIR}/Backend/Classes/*.cpp"
    "${SRC_DIR}/Backend/Database/*.cpp"
    "${SRC_DIR}/Backend/Utilities/*.cpp"
    "${SRC_DIR}/Frontend/*.cpp"
    "${SRC_DIR}/main.cpp"
)
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.h" "${INCLUDE_DIR}/*.hpp")
file(GLOB_RECURSE UI "${FORMS_DIR}/*.ui")
set(RESOURCES "resources.qrc")

# Discord RPC Configuration
option(USE_DISCORD "Enable Discord Rich Presence" ON)

if(WIN32 AND USE_DISCORD)
    # Add RPC files ONLY on Windows
    file(GLOB_RECURSE RPC_SOURCES "${SRC_DIR}/Backend/RPC/*.cpp" "${SRC_DIR}/Backend/RPC/*.c")
    list(APPEND SOURCES ${RPC_SOURCES})
    add_definitions(-DDISCORD_WINDOWS)
else()
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

# --- DEFINE TARGET FIRST ---
add_executable(${PROJECT_NAME}
    ${SOURCES} ${HEADERS} ${UI} ${RESOURCES} ${APP_RC}
)
# ---------------------------

# THEN APPLY TARGET PROPERTIES
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Backend/RPC"
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(USE_DISCORD)
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
file(GLOB_RECURSE BACKEND_SOURCES 
    "${SRC_DIR}/Backend/Classes/*.cpp"
    "${SRC_DIR}/Backend/Database/*.cpp"
    "${SRC_DIR}/Backend/Utilities/*.cpp"
)
file(GLOB_RECURSE TEST_SOURCES "${TESTS_DIR}/*.cpp")

add_executable(${PROJECT_NAME}_tests
    ${BACKEND_SOURCES} ${TEST_SOURCES}
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
