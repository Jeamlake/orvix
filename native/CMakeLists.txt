add_library(
    orvix_capture_core
    STATIC
    src/capture/camera_selector.cpp
    src/capture/continuous_frame_capture.cpp
    src/capture/diagnostic_error.cpp
    src/capture/hresult_error.cpp
    src/capture/media_foundation_camera.cpp
    src/capture/media_foundation_device_enumerator.cpp
    src/capture/media_foundation_runtime.cpp
    src/capture/video_format_selector.cpp
    src/observability/logger.cpp
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
    src/cli/capture_cli.cpp
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

    add_executable(
        camera_selector_tests
        tests/camera_selector_tests.cpp
    )

    target_link_libraries(
        camera_selector_tests
        PRIVATE
        orvix_capture_core
    )

    add_executable(
        media_foundation_camera_tests
        tests/media_foundation_camera_tests.cpp
    )

    target_link_libraries(
        media_foundation_camera_tests
        PRIVATE
        orvix_capture_core
    )

    add_executable(
        video_format_selector_tests
        tests/video_format_selector_tests.cpp
    )

    target_link_libraries(
        video_format_selector_tests
        PRIVATE
        orvix_capture_core
    )

    add_executable(
        continuous_frame_capture_tests
        tests/continuous_frame_capture_tests.cpp
    )

    target_link_libraries(
        continuous_frame_capture_tests
        PRIVATE
        orvix_capture_core
    )

    add_executable(
        logger_tests
        tests/logger_tests.cpp
    )

    target_link_libraries(
        logger_tests
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

        target_compile_options(
            camera_selector_tests
            PRIVATE
            /W4
            /permissive-
            /EHsc
        )

        target_compile_options(
            media_foundation_camera_tests
            PRIVATE
            /W4
            /permissive-
            /EHsc
        )

        target_compile_options(
            video_format_selector_tests
            PRIVATE
            /W4
            /permissive-
            /EHsc
        )

        target_compile_options(
            continuous_frame_capture_tests
            PRIVATE
            /W4
            /permissive-
            /EHsc
        )

        target_compile_options(
            logger_tests
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

    add_test(
        NAME TC_CAP_002_CAMERA_SELECTION
        COMMAND camera_selector_tests
    )

    add_test(
        NAME TC_CAP_002_CLI_INVALID_INDEX
        COMMAND orvix_capture select --index 999999
    )

    add_test(
        NAME TC_CAP_003_MEDIA_FOUNDATION_CAMERA
        COMMAND media_foundation_camera_tests
    )

    add_test(
        NAME TC_CAP_003_CLI_INVALID_INDEX
        COMMAND orvix_capture open --index 999999
    )

    add_test(
        NAME TC_CAP_FORMAT_NEGOTIATION
        COMMAND video_format_selector_tests
    )

    add_test(
        NAME TC_CAP_004_CONTINUOUS_FRAME_CAPTURE
        COMMAND continuous_frame_capture_tests
    )

    add_test(
        NAME TC_CAP_005_FRAME_METADATA
        COMMAND continuous_frame_capture_tests
    )

    add_test(
        NAME TC_MET_001_CAPTURE_FPS
        COMMAND continuous_frame_capture_tests
    )

    add_test(
        NAME TC_CAP_023_CAMERA_STREAM_DIAGNOSTICS
        COMMAND continuous_frame_capture_tests
    )

    add_test(
        NAME TC_CAP_004_CLI_INVALID_INDEX
        COMMAND orvix_capture capture --index 999999
    )

    add_test(
        NAME TC_LOG_001_STRUCTURED_CAPTURE_LOGGING
        COMMAND logger_tests
    )

    set_tests_properties(
        TC_CAP_002_CLI_INVALID_INDEX
        TC_CAP_003_CLI_INVALID_INDEX
        TC_CAP_004_CLI_INVALID_INDEX
        PROPERTIES
        WILL_FAIL TRUE
    )
endif()
