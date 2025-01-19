cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the path to your Qt6 installation
#set(CMAKE_PREFIX_PATH "D:/Qt/6.8.1/mingw_64/lib/cmake")

# Enable Qt's automatic features for the main executable
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

project(MindLeap)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Multimedia)

# Paths for source files and forms
set(FORMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

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
        AUTOUIC_SEARCH_PATHS ${FORMS_DIR}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Multimedia)

# Enable compiler optimizations and link-time optimization (LTO)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")

# Copy sound files to the build directory
file(GLOB_RECURSE SOUNDS "${ASSETS_DIR}/sounds/*.wav")
foreach(SOUND ${SOUNDS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SOUND}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

# Add Catch2 for tests
include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.0
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
        Catch2::Catch2WithMain
        Qt6::Core
        Qt6::Sql
)

# Platform-specific deployment commands
if(WIN32)
    # For Windows: Find and use windeployqt
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt.exe
            PATHS $ENV{QTDIR}/bin D:/Qt/6.8.1/mingw_64/bin
            NO_DEFAULT_PATH
    )

    if(NOT WINDEPLOYQT_EXECUTABLE)
        message(FATAL_ERROR "windeployqt.exe not found! Please ensure Qt is correctly installed and the path is set.")
    else()
        message(STATUS "windeployqt found at: ${WINDEPLOYQT_EXECUTABLE}")
    endif()

    # Deploy for the main executable
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --dir $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Running windeployqt for ${PROJECT_NAME}"
    )

    # Deploy for the test executable
    add_custom_command(TARGET ${PROJECT_NAME}_tests POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --dir $<TARGET_FILE_DIR:${PROJECT_NAME}_tests> $<TARGET_FILE:${PROJECT_NAME}_tests>
            COMMENT "Running windeployqt for ${PROJECT_NAME}_tests"
    )

elseif(APPLE)
    # For macOS: Find and use macdeployqt
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt
            PATHS $ENV{QTDIR}/bin /usr/local/opt/qt/bin
            NO_DEFAULT_PATH
    )

    if(NOT MACDEPLOYQT_EXECUTABLE)
        message(FATAL_ERROR "macdeployqt not found! Please ensure Qt is correctly installed and the path is set.")
    else()
        message(STATUS "macdeployqt found at: ${MACDEPLOYQT_EXECUTABLE}")
    endif()

    # Deploy for the main executable
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${MACDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Running macdeployqt for ${PROJECT_NAME}"
    )

    # Deploy for the test executable
    add_custom_command(TARGET ${PROJECT_NAME}_tests POST_BUILD
            COMMAND ${MACDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}_tests}>
            COMMENT "Running macdeployqt for ${PROJECT_NAME}_tests"
    )

elseif(UNIX)
    # For Linux: Find and use linuxdeployqt
    find_program(LINUXDEPLOYQT_EXECUTABLE linuxdeployqt
            PATHS $ENV{QTDIR}/bin /opt/qt5/bin /usr/local/qt5/bin
            NO_DEFAULT_PATH
    )

    if(NOT LINUXDEPLOYQT_EXECUTABLE)
        message(FATAL_ERROR "linuxdeployqt not found! Please ensure Qt is correctly installed and the path is set.")
    else()
        message(STATUS "linuxdeployqt found at: ${LINUXDEPLOYQT_EXECUTABLE}")
    endif()

    # Deploy for the main executable
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${LINUXDEPLOYQT_EXECUTABLE} --dir $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Running linuxdeployqt for ${PROJECT_NAME}"
    )

    # Deploy for the test executable
    add_custom_command(TARGET ${PROJECT_NAME}_tests POST_BUILD
            COMMAND ${LINUXDEPLOYQT_EXECUTABLE} --dir $<TARGET_FILE_DIR:${PROJECT_NAME}_tests> $<TARGET_FILE:${PROJECT_NAME}_tests>
            COMMENT "Running linuxdeployqt for ${PROJECT_NAME}_tests"
    )
endif()

# Enable CTest
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME}_tests)