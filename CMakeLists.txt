cmake_minimum_required(VERSION 3.20)

project(
    ORVIX
    VERSION 0.1.0
    DESCRIPTION "Optical Real-time Video Ingestion eXchange"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    ORVIX_BUILD_TESTS
    "Build ORVIX tests"
    ON
)

if(ORVIX_BUILD_TESTS)
    include(CTest)
    enable_testing()
endif()

add_subdirectory(native)