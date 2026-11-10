# Establece la variable de CMake.
set(
    # Incorpora ORVIX_CAPTURE_CORE_SOURCES en esta operación.
    ORVIX_CAPTURE_CORE_SOURCES
    # Incorpora src/capture/camera_selector.cpp a la definición del objetivo.
    src/capture/camera_selector.cpp
    # Incorpora src/capture/continuous_frame_capture.cpp a la definición del objetivo.
    src/capture/continuous_frame_capture.cpp
    # Incorpora src/capture/diagnostic_error.cpp a la definición del objetivo.
    src/capture/diagnostic_error.cpp
    # Incorpora src/capture/platform_factory.cpp a la definición del objetivo.
    src/capture/platform_factory.cpp
    # Incorpora src/capture/synthetic_frame_reader.cpp a la definición del objetivo.
    src/capture/synthetic_frame_reader.cpp
    # Incorpora src/capture/video_format_selector.cpp a la definición del objetivo.
    src/capture/video_format_selector.cpp
    # Incorpora src/ipc/shared_memory_publisher.cpp a la definición del objetivo.
    src/ipc/shared_memory_publisher.cpp
    # Incorpora src/ipc/shared_memory_region.cpp a la definición del objetivo.
    src/ipc/shared_memory_region.cpp
    # Incorpora src/observability/logger.cpp a la definición del objetivo.
    src/observability/logger.cpp
)

# Abre una rama de configuración cuando se cumple WIN32.
if(WIN32)
    # Ejecuta la orden list.
    list(
        # Incorpora APPEND ORVIX_CAPTURE_CORE_SOURCES en esta operación.
        APPEND ORVIX_CAPTURE_CORE_SOURCES
        # Incorpora src/capture/hresult_error.cpp a la definición del objetivo.
        src/capture/hresult_error.cpp
        # Incorpora src/capture/media_foundation_camera.cpp a la definición del objetivo.
        src/capture/media_foundation_camera.cpp
        # Incorpora src/capture/media_foundation_device_enumerator.cpp a la definición del objetivo.
        src/capture/media_foundation_device_enumerator.cpp
        # Incorpora src/capture/media_foundation_runtime.cpp a la definición del objetivo.
        src/capture/media_foundation_runtime.cpp
    )
# Comprueba la alternativa de configuración APPLE.
elseif(APPLE)
    # Ejecuta la orden list.
    list(
        # Incorpora APPEND ORVIX_CAPTURE_CORE_SOURCES en esta operación.
        APPEND ORVIX_CAPTURE_CORE_SOURCES
        # Incorpora src/capture/av_foundation_camera.mm a la definición del objetivo.
        src/capture/av_foundation_camera.mm
        # Incorpora src/capture/av_foundation_device_enumerator.mm a la definición del objetivo.
        src/capture/av_foundation_device_enumerator.mm
    )
# Comprueba la alternativa de configuración CMAKE_SYSTEM_NAME STREQUAL "Linux".
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Ejecuta la orden list.
    list(
        # Incorpora APPEND ORVIX_CAPTURE_CORE_SOURCES en esta operación.
        APPEND ORVIX_CAPTURE_CORE_SOURCES
        # Incorpora src/capture/v4l2_camera.cpp a la definición del objetivo.
        src/capture/v4l2_camera.cpp
        # Incorpora src/capture/v4l2_device_enumerator.cpp a la definición del objetivo.
        src/capture/v4l2_device_enumerator.cpp
    )
# Abre la alternativa de configuración.
else()
    # Emite el diagnóstico de configuración FATAL_ERROR "ORVIX supports Windows, Linux, and macOS.".
    message(FATAL_ERROR "ORVIX supports Windows, Linux, and macOS.")
# Cierra la rama de configuración.
endif()

# Declara la biblioteca nativa.
add_library(
    # Incorpora orvix_capture_core en esta operación.
    orvix_capture_core
    # Incorpora STATIC en esta operación.
    STATIC
    # Incorpora ${ORVIX_CAPTURE_CORE_SOURCES} en esta operación.
    ${ORVIX_CAPTURE_CORE_SOURCES}
)

# Configura las rutas de cabeceras del objetivo.
target_include_directories(
    # Incorpora orvix_capture_core en esta operación.
    orvix_capture_core
    # Aplica las opciones al objetivo y sus consumidores.
    PUBLIC
    # Incorpora ${CMAKE_CURRENT_SOURCE_DIR}/include en esta operación.
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Ejecuta la orden target_compile_features.
target_compile_features(
    # Incorpora orvix_capture_core en esta operación.
    orvix_capture_core
    # Aplica las opciones al objetivo y sus consumidores.
    PUBLIC
    # Incorpora cxx_std_20 en esta operación.
    cxx_std_20
)

# Abre una rama de configuración cuando se cumple MSVC.
if(MSVC)
    # Pasa las opciones al compilador.
    target_compile_options(
        # Incorpora orvix_capture_core en esta operación.
        orvix_capture_core
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora /W4 en esta operación.
        /W4
        # Incorpora /permissive- en esta operación.
        /permissive-
        # Incorpora /EHsc en esta operación.
        /EHsc
    )
# Abre la alternativa de configuración.
else()
    # Pasa las opciones al compilador.
    target_compile_options(
        # Incorpora orvix_capture_core en esta operación.
        orvix_capture_core
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora -Wall en esta operación.
        -Wall
        # Incorpora -Wextra en esta operación.
        -Wextra
        # Incorpora -Wpedantic en esta operación.
        -Wpedantic
    )
# Cierra la rama de configuración.
endif()

# Abre una rama de configuración cuando se cumple WIN32.
if(WIN32)
    # Abre una rama de configuración cuando se cumple ORVIX_WINDOWS_7_COMPAT.
    if(ORVIX_WINDOWS_7_COMPAT)
        # Establece la variable de CMake ORVIX_WINDOWS_VERSION 0x0601.
        set(ORVIX_WINDOWS_VERSION 0x0601)
    # Abre la alternativa de configuración.
    else()
        # Establece la variable de CMake ORVIX_WINDOWS_VERSION 0x0A00.
        set(ORVIX_WINDOWS_VERSION 0x0A00)
    # Cierra la rama de configuración.
    endif()

    # Expone definiciones al compilador.
    target_compile_definitions(
        # Incorpora orvix_capture_core en esta operación.
        orvix_capture_core
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora WIN32_LEAN_AND_MEAN en esta operación.
        WIN32_LEAN_AND_MEAN
        # Incorpora NOMINMAX en esta operación.
        NOMINMAX
        # Incorpora WINVER=${ORVIX_WINDOWS_VERSION} en esta operación.
        WINVER=${ORVIX_WINDOWS_VERSION}
        # Incorpora _WIN32_WINNT=${ORVIX_WINDOWS_VERSION} en esta operación.
        _WIN32_WINNT=${ORVIX_WINDOWS_VERSION}
    )

    # Enlaza las dependencias indicadas.
    target_link_libraries(
        # Incorpora orvix_capture_core en esta operación.
        orvix_capture_core
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora mf en esta operación.
        mf
        # Incorpora mfplat en esta operación.
        mfplat
        # Incorpora mfreadwrite en esta operación.
        mfreadwrite
        # Incorpora mfuuid en esta operación.
        mfuuid
        # Incorpora ole32 en esta operación.
        ole32
    )
# Comprueba la alternativa de configuración APPLE.
elseif(APPLE)
    # Ejecuta la orden find_library AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED.
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED)
    # Ejecuta la orden find_library COREMEDIA_FRAMEWORK CoreMedia REQUIRED.
    find_library(COREMEDIA_FRAMEWORK CoreMedia REQUIRED)
    # Ejecuta la orden find_library COREVIDEO_FRAMEWORK CoreVideo REQUIRED.
    find_library(COREVIDEO_FRAMEWORK CoreVideo REQUIRED)
    # Ejecuta la orden find_library FOUNDATION_FRAMEWORK Foundation REQUIRED.
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)

    # Ejecuta la orden set_source_files_properties.
    set_source_files_properties(
        # Incorpora src/capture/av_foundation_camera.mm a la definición del objetivo.
        src/capture/av_foundation_camera.mm
        # Incorpora src/capture/av_foundation_device_enumerator.mm a la definición del objetivo.
        src/capture/av_foundation_device_enumerator.mm
        # Incorpora PROPERTIES en esta operación.
        PROPERTIES
        # Incorpora COMPILE_OPTIONS "-fobjc-arc" en esta operación.
        COMPILE_OPTIONS "-fobjc-arc"
    )

    # Enlaza las dependencias indicadas.
    target_link_libraries(
        # Incorpora orvix_capture_core en esta operación.
        orvix_capture_core
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora ${AVFOUNDATION_FRAMEWORK} en esta operación.
        ${AVFOUNDATION_FRAMEWORK}
        # Incorpora ${COREMEDIA_FRAMEWORK} en esta operación.
        ${COREMEDIA_FRAMEWORK}
        # Incorpora ${COREVIDEO_FRAMEWORK} en esta operación.
        ${COREVIDEO_FRAMEWORK}
        # Incorpora ${FOUNDATION_FRAMEWORK} en esta operación.
        ${FOUNDATION_FRAMEWORK}
    )
# Comprueba la alternativa de configuración CMAKE_SYSTEM_NAME STREQUAL "Linux".
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Enlaza las dependencias indicadas.
    target_link_libraries(
        # Incorpora orvix_capture_core en esta operación.
        orvix_capture_core
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora rt en esta operación.
        rt
    )
# Cierra la rama de configuración.
endif()

# Declara el ejecutable que se compilará.
add_executable(
    # Incorpora orvix_capture en esta operación.
    orvix_capture
    # Incorpora src/cli/capture_cli.cpp a la definición del objetivo.
    src/cli/capture_cli.cpp
    # Incorpora src/main.cpp a la definición del objetivo.
    src/main.cpp
)

# Ajusta las propiedades del objetivo.
set_target_properties(
    # Incorpora orvix_capture en esta operación.
    orvix_capture
    # Incorpora PROPERTIES en esta operación.
    PROPERTIES
    # Incorpora OUTPUT_NAME "orvix-capture" en esta operación.
    OUTPUT_NAME "orvix-capture"
)

# Enlaza las dependencias indicadas.
target_link_libraries(
    # Incorpora orvix_capture en esta operación.
    orvix_capture
    # Aplica las opciones solo al objetivo.
    PRIVATE
    # Incorpora orvix_capture_core en esta operación.
    orvix_capture_core
)

# Abre una rama de configuración cuando se cumple APPLE.
if(APPLE)
    # Pasa las opciones al enlazador.
    target_link_options(
        # Incorpora orvix_capture en esta operación.
        orvix_capture
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora "-Wl,-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist" a la definición del objetivo.
        "-Wl,-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist"
    )
# Cierra la rama de configuración.
endif()

# Abre una rama de configuración cuando se cumple MSVC.
if(MSVC)
    # Pasa las opciones al compilador.
    target_compile_options(
        # Incorpora orvix_capture en esta operación.
        orvix_capture
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora /W4 en esta operación.
        /W4
        # Incorpora /permissive- en esta operación.
        /permissive-
        # Incorpora /EHsc en esta operación.
        /EHsc
    )
# Abre la alternativa de configuración.
else()
    # Pasa las opciones al compilador.
    target_compile_options(
        # Incorpora orvix_capture en esta operación.
        orvix_capture
        # Aplica las opciones solo al objetivo.
        PRIVATE
        # Incorpora -Wall en esta operación.
        -Wall
        # Incorpora -Wextra en esta operación.
        -Wextra
        # Incorpora -Wpedantic en esta operación.
        -Wpedantic
    )
# Cierra la rama de configuración.
endif()

# Abre una rama de configuración cuando se cumple ORVIX_BUILD_TESTS.
if(ORVIX_BUILD_TESTS)
    # Establece la variable de CMake.
    set(
        # Incorpora ORVIX_TEST_TARGETS en esta operación.
        ORVIX_TEST_TARGETS
        # Incorpora camera_device_tests en esta operación.
        camera_device_tests
        # Incorpora camera_selector_tests en esta operación.
        camera_selector_tests
        # Incorpora continuous_frame_capture_tests en esta operación.
        continuous_frame_capture_tests
        # Incorpora logger_tests en esta operación.
        logger_tests
        # Incorpora platform_factory_tests en esta operación.
        platform_factory_tests
        # Incorpora shared_memory_publisher_tests en esta operación.
        shared_memory_publisher_tests
        # Incorpora synthetic_frame_reader_tests en esta operación.
        synthetic_frame_reader_tests
        # Incorpora video_format_selector_tests en esta operación.
        video_format_selector_tests
    )

    # Ejecuta la orden foreach test_target IN LISTS ORVIX_TEST_TARGETS.
    foreach(test_target IN LISTS ORVIX_TEST_TARGETS)
        # Declara el ejecutable que se compilará.
        add_executable(
            # Incorpora ${test_target} en esta operación.
            ${test_target}
            # Incorpora tests/${test_target}.cpp a la definición del objetivo.
            tests/${test_target}.cpp
        )

        # Enlaza las dependencias indicadas.
        target_link_libraries(
            # Incorpora ${test_target} en esta operación.
            ${test_target}
            # Aplica las opciones solo al objetivo.
            PRIVATE
            # Incorpora orvix_capture_core en esta operación.
            orvix_capture_core
        )

        # Abre una rama de configuración cuando se cumple MSVC.
        if(MSVC)
            # Pasa las opciones al compilador.
            target_compile_options(
                # Incorpora ${test_target} en esta operación.
                ${test_target}
                # Aplica las opciones solo al objetivo.
                PRIVATE
                # Incorpora /W4 en esta operación.
                /W4
                # Incorpora /permissive- en esta operación.
                /permissive-
                # Incorpora /EHsc en esta operación.
                /EHsc
            )
        # Abre la alternativa de configuración.
        else()
            # Pasa las opciones al compilador.
            target_compile_options(
                # Incorpora ${test_target} en esta operación.
                ${test_target}
                # Aplica las opciones solo al objetivo.
                PRIVATE
                # Incorpora -Wall en esta operación.
                -Wall
                # Incorpora -Wextra en esta operación.
                -Wextra
                # Incorpora -Wpedantic en esta operación.
                -Wpedantic
            )
        # Cierra la rama de configuración.
        endif()
    # Ejecuta la orden endforeach.
    endforeach()

    # Abre una rama de configuración cuando se cumple WIN32.
    if(WIN32)
        # Declara el ejecutable que se compilará.
        add_executable(
            # Incorpora media_foundation_camera_tests en esta operación.
            media_foundation_camera_tests
            # Incorpora tests/media_foundation_camera_tests.cpp a la definición del objetivo.
            tests/media_foundation_camera_tests.cpp
        )

        # Enlaza las dependencias indicadas.
        target_link_libraries(
            # Incorpora media_foundation_camera_tests en esta operación.
            media_foundation_camera_tests
            # Aplica las opciones solo al objetivo.
            PRIVATE
            # Incorpora orvix_capture_core en esta operación.
            orvix_capture_core
        )

        # Pasa las opciones al compilador.
        target_compile_options(
            # Incorpora media_foundation_camera_tests en esta operación.
            media_foundation_camera_tests
            # Aplica las opciones solo al objetivo.
            PRIVATE
            # Incorpora /W4 en esta operación.
            /W4
            # Incorpora /permissive- en esta operación.
            /permissive-
            # Incorpora /EHsc en esta operación.
            /EHsc
        )
    # Cierra la rama de configuración.
    endif()

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_001_CAMERA_DEVICE_MODEL en esta operación.
        NAME TC_CAP_001_CAMERA_DEVICE_MODEL
        # Incorpora COMMAND camera_device_tests en esta operación.
        COMMAND camera_device_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_001_CLI_HELP en esta operación.
        NAME TC_CAP_001_CLI_HELP
        # Incorpora COMMAND orvix_capture --help en esta operación.
        COMMAND orvix_capture --help
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_001_CLI_DEVICE_ENUMERATION en esta operación.
        NAME TC_CAP_001_CLI_DEVICE_ENUMERATION
        # Incorpora COMMAND orvix_capture devices en esta operación.
        COMMAND orvix_capture devices
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_002_CAMERA_SELECTION en esta operación.
        NAME TC_CAP_002_CAMERA_SELECTION
        # Incorpora COMMAND camera_selector_tests en esta operación.
        COMMAND camera_selector_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_002_CLI_INVALID_INDEX en esta operación.
        NAME TC_CAP_002_CLI_INVALID_INDEX
        # Incorpora COMMAND orvix_capture select --index 999999 en esta operación.
        COMMAND orvix_capture select --index 999999
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_003_PLATFORM_FACTORY en esta operación.
        NAME TC_CAP_003_PLATFORM_FACTORY
        # Incorpora COMMAND platform_factory_tests en esta operación.
        COMMAND platform_factory_tests
    )

    # Abre una rama de configuración cuando se cumple WIN32.
    if(WIN32)
        # Registra el caso de prueba para CTest.
        add_test(
            # Incorpora NAME TC_CAP_003_MEDIA_FOUNDATION_CAMERA en esta operación.
            NAME TC_CAP_003_MEDIA_FOUNDATION_CAMERA
            # Incorpora COMMAND media_foundation_camera_tests en esta operación.
            COMMAND media_foundation_camera_tests
        )
    # Cierra la rama de configuración.
    endif()

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_FORMAT_NEGOTIATION en esta operación.
        NAME TC_CAP_FORMAT_NEGOTIATION
        # Incorpora COMMAND video_format_selector_tests en esta operación.
        COMMAND video_format_selector_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_004_CONTINUOUS_FRAME_CAPTURE en esta operación.
        NAME TC_CAP_004_CONTINUOUS_FRAME_CAPTURE
        # Incorpora COMMAND continuous_frame_capture_tests en esta operación.
        COMMAND continuous_frame_capture_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_005_FRAME_METADATA en esta operación.
        NAME TC_CAP_005_FRAME_METADATA
        # Incorpora COMMAND continuous_frame_capture_tests en esta operación.
        COMMAND continuous_frame_capture_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_MET_001_CAPTURE_FPS en esta operación.
        NAME TC_MET_001_CAPTURE_FPS
        # Incorpora COMMAND continuous_frame_capture_tests en esta operación.
        COMMAND continuous_frame_capture_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_023_CAMERA_STREAM_DIAGNOSTICS en esta operación.
        NAME TC_CAP_023_CAMERA_STREAM_DIAGNOSTICS
        # Incorpora COMMAND continuous_frame_capture_tests en esta operación.
        COMMAND continuous_frame_capture_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_CAP_004_CLI_INVALID_INDEX en esta operación.
        NAME TC_CAP_004_CLI_INVALID_INDEX
        # Incorpora COMMAND orvix_capture capture --index 999999 en esta operación.
        COMMAND orvix_capture capture --index 999999
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_LOG_001_STRUCTURED_CAPTURE_LOGGING en esta operación.
        NAME TC_LOG_001_STRUCTURED_CAPTURE_LOGGING
        # Incorpora COMMAND logger_tests en esta operación.
        COMMAND logger_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_IPC_001_SHARED_MEMORY_PUBLICATION en esta operación.
        NAME TC_IPC_001_SHARED_MEMORY_PUBLICATION
        # Incorpora COMMAND shared_memory_publisher_tests en esta operación.
        COMMAND shared_memory_publisher_tests
    )

    # Registra el caso de prueba para CTest.
    add_test(
        # Incorpora NAME TC_SYN_001_SYNTHETIC_FRAME_SOURCE en esta operación.
        NAME TC_SYN_001_SYNTHETIC_FRAME_SOURCE
        # Incorpora COMMAND synthetic_frame_reader_tests en esta operación.
        COMMAND synthetic_frame_reader_tests
    )

    # Ejecuta la orden set_tests_properties.
    set_tests_properties(
        # Incorpora TC_CAP_002_CLI_INVALID_INDEX en esta operación.
        TC_CAP_002_CLI_INVALID_INDEX
        # Incorpora TC_CAP_004_CLI_INVALID_INDEX en esta operación.
        TC_CAP_004_CLI_INVALID_INDEX
        # Incorpora PROPERTIES en esta operación.
        PROPERTIES
        # Incorpora WILL_FAIL TRUE en esta operación.
        WILL_FAIL TRUE
    )
# Cierra la rama de configuración.
endif()
