cmake_minimum_required(VERSION 3.20)

project(
    ORVIX
    VERSION 0.1.0
    DESCRIPTION "Optical Real-time Video Ingestion eXchange"
    LANGUAGES CXX
)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    ORVIX_BUILD_TESTS
    "Build ORVIX tests"
    ON
)

option(
    ORVIX_WINDOWS_7_COMPAT
    "Restrict the Windows build to Windows 7 SP1-compatible APIs and static MSVC runtime"
    ON
)

if(MSVC AND ORVIX_WINDOWS_7_COMPAT)
    set(
        CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()

if(ORVIX_BUILD_TESTS)
    include(CTest)
    enable_testing()
endif()

add_subdirectory(native)
