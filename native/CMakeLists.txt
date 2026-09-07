set(
    ORVIX_CAPTURE_CORE_SOURCES
    src/capture/camera_selector.cpp
    src/capture/continuous_frame_capture.cpp
    src/capture/diagnostic_error.cpp
    src/capture/platform_factory.cpp
    src/capture/video_format_selector.cpp
    src/observability/logger.cpp
)

if(WIN32)
    list(
        APPEND ORVIX_CAPTURE_CORE_SOURCES
        src/capture/hresult_error.cpp
        src/capture/media_foundation_camera.cpp
        src/capture/media_foundation_device_enumerator.cpp
        src/capture/media_foundation_runtime.cpp
    )
elseif(APPLE)
    list(
        APPEND ORVIX_CAPTURE_CORE_SOURCES
        src/capture/av_foundation_camera.mm
        src/capture/av_foundation_device_enumerator.mm
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(
        APPEND ORVIX_CAPTURE_CORE_SOURCES
        src/capture/v4l2_camera.cpp
        src/capture/v4l2_device_enumerator.cpp
    )
else()
    message(FATAL_ERROR "ORVIX supports Windows, Linux, and macOS.")
endif()

add_library(
    orvix_capture_core
    STATIC
    ${ORVIX_CAPTURE_CORE_SOURCES}
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
else()
    target_compile_options(
        orvix_capture_core
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

if(WIN32)
    target_compile_definitions(
        orvix_capture_core
        PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0601
    )

    target_link_libraries(
        orvix_capture_core
        PRIVATE
        mf
        mfplat
        mfreadwrite
        mfuuid
        ole32
    )
elseif(APPLE)
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED)
    find_library(COREMEDIA_FRAMEWORK CoreMedia REQUIRED)
    find_library(COREVIDEO_FRAMEWORK CoreVideo REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)

    set_source_files_properties(
        src/capture/av_foundation_camera.mm
        src/capture/av_foundation_device_enumerator.mm
        PROPERTIES
        COMPILE_OPTIONS "-fobjc-arc"
    )

    target_link_libraries(
        orvix_capture_core
        PRIVATE
        ${AVFOUNDATION_FRAMEWORK}
        ${COREMEDIA_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
    )
endif()

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

if(APPLE)
    target_link_options(
        orvix_capture
        PRIVATE
        "-Wl,-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist"
    )
endif()

if(MSVC)
    target_compile_options(
        orvix_capture
        PRIVATE
        /W4
        /permissive-
        /EHsc
    )
else()
    target_compile_options(
        orvix_capture
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

if(ORVIX_BUILD_TESTS)
    set(
        ORVIX_TEST_TARGETS
        camera_device_tests
        camera_selector_tests
        continuous_frame_capture_tests
        logger_tests
        platform_factory_tests
        video_format_selector_tests
    )

    foreach(test_target IN LISTS ORVIX_TEST_TARGETS)
        add_executable(
            ${test_target}
            tests/${test_target}.cpp
        )

        target_link_libraries(
            ${test_target}
            PRIVATE
            orvix_capture_core
        )

        if(MSVC)
            target_compile_options(
                ${test_target}
                PRIVATE
                /W4
                /permissive-
                /EHsc
            )
        else()
            target_compile_options(
                ${test_target}
                PRIVATE
                -Wall
                -Wextra
                -Wpedantic
            )
        endif()
    endforeach()

    if(WIN32)
        add_executable(
            media_foundation_camera_tests
            tests/media_foundation_camera_tests.cpp
        )

        target_link_libraries(
            media_foundation_camera_tests
            PRIVATE
            orvix_capture_core
        )

        target_compile_options(
            media_foundation_camera_tests
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
        NAME TC_CAP_003_PLATFORM_FACTORY
        COMMAND platform_factory_tests
    )

    if(WIN32)
        add_test(
            NAME TC_CAP_003_MEDIA_FOUNDATION_CAMERA
            COMMAND media_foundation_camera_tests
        )
    endif()

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
        TC_CAP_004_CLI_INVALID_INDEX
        PROPERTIES
        WILL_FAIL TRUE
    )
endif()
