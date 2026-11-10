# Fija la versión mínima de CMake VERSION 3.20.
cmake_minimum_required(VERSION 3.20)

# Declara el proyecto y sus lenguajes de compilación.
project(
    # Incorpora ORVIX en esta operación.
    ORVIX
    # Incorpora VERSION 1.0.0 en esta operación.
    VERSION 1.0.0
    # Incorpora DESCRIPTION "Optical Real-time Video Ingestion eXchange" en esta operación.
    DESCRIPTION "Optical Real-time Video Ingestion eXchange"
    # Incorpora LANGUAGES CXX en esta operación.
    LANGUAGES CXX
)

# Abre una rama de configuración cuando se cumple APPLE.
if(APPLE)
    # Habilita un lenguaje para los objetivos nativos OBJCXX.
    enable_language(OBJCXX)
# Cierra la rama de configuración.
endif()

# Establece la variable de CMake CMAKE_CXX_STANDARD 20.
set(CMAKE_CXX_STANDARD 20)
# Establece la variable de CMake CMAKE_CXX_STANDARD_REQUIRED ON.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Establece la variable de CMake CMAKE_CXX_EXTENSIONS OFF.
set(CMAKE_CXX_EXTENSIONS OFF)

# Declara una opción configurable de compilación.
option(
    # Incorpora ORVIX_BUILD_TESTS en esta operación.
    ORVIX_BUILD_TESTS
    # Aporta el texto "Build ORVIX tests" a la expresión.
    "Build ORVIX tests"
    # Incorpora ON en esta operación.
    ON
)

# Declara una opción configurable de compilación.
option(
    # Incorpora ORVIX_WINDOWS_7_COMPAT en esta operación.
    ORVIX_WINDOWS_7_COMPAT
    # Aporta el texto "Restrict the Windows build to Windows 7 SP1-compatible APIs and static MSVC runtime" a la expresión.
    "Restrict the Windows build to Windows 7 SP1-compatible APIs and static MSVC runtime"
    # Incorpora ON en esta operación.
    ON
)

# Abre una rama de configuración cuando se cumple MSVC AND ORVIX_WINDOWS_7_COMPAT.
if(MSVC AND ORVIX_WINDOWS_7_COMPAT)
    # Establece la variable de CMake.
    set(
        # Incorpora CMAKE_MSVC_RUNTIME_LIBRARY en esta operación.
        CMAKE_MSVC_RUNTIME_LIBRARY
        # Aporta el texto "MultiThreaded$<$<CONFIG:Debug>:Debug>" a la expresión.
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
# Cierra la rama de configuración.
endif()

# Abre una rama de configuración cuando se cumple ORVIX_BUILD_TESTS.
if(ORVIX_BUILD_TESTS)
    # Carga el módulo de CMake CTest.
    include(CTest)
    # Activa el registro de las pruebas.
    enable_testing()
# Cierra la rama de configuración.
endif()

# Carga la configuración del subdirectorio src/native native.
add_subdirectory(src/native native)
