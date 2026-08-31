add_library(
    orvix_capture_core
    STATIC
    src/capture/hresult_error.cpp
    src/capture/media_foundation_device_enumerator.cpp
)

target_include_directories(
    orvix_capture_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
    orvix_capture_core
    PUBLIC
    cxx_std_20
)

if(MSVC)
    target_compile_options(
        orvix_capture_core
        PRIVATE
        /W4
        /permissive-
        /EHsc
    )
endif()

target_link_libraries(
    orvix_capture_core
    PRIVATE
    mf
    mfplat
    mfreadwrite
    mfuuid
    ole32
)

add_executable(
    orvix_capture
    src/main.cpp
)

set_target_properties(
    orvix_capture
    PROPERTIES
    OUTPUT_NAME "orvix-capture"
)

target_link_libraries(
    orvix_capture
    PRIVATE
    orvix_capture_core
)

if(MSVC)
    target_compile_options(
        orvix_capture
        PRIVATE
        /W4
        /permissive-
        /EHsc
    )
endif()

if(ORVIX_BUILD_TESTS)
    add_executable(
        camera_device_tests
        tests/camera_device_tests.cpp
    )

    target_link_libraries(
        camera_device_tests
        PRIVATE
        orvix_capture_core
    )

    if(MSVC)
        target_compile_options(
            camera_device_tests
            PRIVATE
            /W4
            /permissive-
            /EHsc
        )
    endif()

    add_test(
        NAME TC_CAP_001_CAMERA_DEVICE_MODEL
        COMMAND camera_device_tests
    )

    add_test(
        NAME TC_CAP_001_CLI_HELP
        COMMAND orvix_capture --help
    )
endif()