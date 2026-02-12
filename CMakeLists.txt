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

# Completely exclude RPC implementation files from global SOURCES
file(GLOB_RECURSE RPC_INTERNAL_SOURCES "${SRC_DIR}/Backend/RPC/*.cpp")
if(RPC_INTERNAL_SOURCES)
    list(REMOVE_ITEM SOURCES ${RPC_INTERNAL_SOURCES})
endif()

# Discord RPC Configuration
option(USE_DISCORD "Enable Discord Rich Presence" ON)
if(USE_DISCORD AND WIN32)
    add_definitions(-DDISCORD_WINDOWS)
    # Add implementation files back ONLY on Windows
    list(APPEND SOURCES 
        "${SRC_DIR}/Backend/RPC/discord_rpc.cpp"
        "${SRC_DIR}/Backend/RPC/rpc_connection.cpp"
        "${SRC_DIR}/Backend/RPC/serialization.cpp"
        "${SRC_DIR}/Backend/RPC/connection_win.cpp"
        "${SRC_DIR}/Backend/RPC/discord_register_win.cpp"
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DISCORD_RPC)
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

add_executable(${PROJECT_NAME}
    ${SOURCES} ${HEADERS} ${UI} ${RESOURCES} ${APP_RC}
)

# Include directories
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

# ... rest of your CMakeLists.txt (tests) remains unchanged

