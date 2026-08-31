add_library(orvix_core INTERFACE)

target_include_directories(
    orvix_core
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(orvix_core INTERFACE cxx_std_20)

if(MSVC)
    target_compile_options(orvix_core INTERFACE /W4 /permissive-)
endif()