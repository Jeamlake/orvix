// Incluye las declaraciones de shared memory region de ORVIX.
#include "orvix/ipc/shared_memory_region.hpp"

// Incluye algorithm para usar algoritmos sobre rangos.
#include <algorithm>
// Importa las declaraciones de cctype para esta unidad.
#include <cctype>
// Incluye cerrno para usar códigos de error del sistema.
#include <cerrno>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye limits para usar límites de tipos numéricos.
#include <limits>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Importa las declaraciones de system_error para esta unidad.
#include <system_error>
// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>

// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
// Importa las declaraciones de Windows.h para esta unidad.
#include <Windows.h>
// Abre la alternativa de compilación.
#else
// Incluye fcntl.h para usar apertura y opciones de descriptores.
#include <fcntl.h>
// Incluye sys/mman.h para usar mapeo de memoria.
#include <sys/mman.h>
// Importa las declaraciones de sys/stat.h para esta unidad.
#include <sys/stat.h>
// Incluye unistd.h para usar operaciones POSIX.
#include <unistd.h>
// Cierra la condición de compilación.
#endif

// Agrupa las declaraciones bajo orvix::ipc.
namespace orvix::ipc {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Define la operación validate_name con los parámetros declarados.
void validate_name(const std::string& name) {
    // Evalúa la condición name.empty() || name.size() > 30.
    if (name.empty() || name.size() > 30) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "Shared-memory names must contain between 1 and 30 characters." a la expresión.
            "Shared-memory names must contain between 1 and 30 characters."
        );
    }
    // Establece valid a partir de std::all_of(.
    const bool valid = std::all_of(
        // Ejecuta name.begin(),.
        name.begin(),
        // Ejecuta name.end(),.
        name.end(),
        // Envía el mensaje de Objective-C indicado por [](const unsigned char character) {.
        [](const unsigned char character) {
            // Devuelve el resultado de std::isalnum(character) != 0 ||.
            return
                // Combina esta condición con std::isalnum(character) != 0 ||.
                std::isalnum(character) != 0 ||
                // Combina esta condición con character == '-' ||.
                character == '-' ||
                // Evalúa character == '_' como parte de la condición.
                character == '_';
        }
    );
    // Evalúa la condición !valid.
    if (!valid) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "Shared-memory names may contain letters, digits, '-' and '_'." a la expresión.
            "Shared-memory names may contain letters, digits, '-' and '_'."
        );
    }
}

// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)

// Define la operación utf8_to_wide con los parámetros declarados.
std::wstring utf8_to_wide(const std::string& value) {
    // Establece required a partir de MultiByteToWideChar(.
    const int required = MultiByteToWideChar(
        // Aporta CP_UTF8 como elemento de la lista.
        CP_UTF8,
        // Aporta MB_ERR_INVALID_CHARS como elemento de la lista.
        MB_ERR_INVALID_CHARS,
        // Ejecuta value.c_str(),.
        value.c_str(),
        // Aporta -1 como elemento de la lista.
        -1,
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Incorpora 0 en esta operación.
        0
    );
    // Evalúa la condición required <= 0.
    if (required <= 0) {
        // Interrumpe la operación con std::system_error(.
        throw std::system_error(
            // Ejecuta static_cast<int>(GetLastError()),.
            static_cast<int>(GetLastError()),
            // Ejecuta std::system_category(),.
            std::system_category(),
            // Aporta el texto "MultiByteToWideChar(shared-memory name)" a la expresión.
            "MultiByteToWideChar(shared-memory name)"
        );
    }
    // Ejecuta std::wstring result(static_cast<std::size_t>(required), L'\0').
    std::wstring result(static_cast<std::size_t>(required), L'\0');
    // Evalúa la condición MultiByteToWideChar(.
    if (MultiByteToWideChar(
        // Aporta CP_UTF8 como elemento de la lista.
        CP_UTF8,
        // Aporta MB_ERR_INVALID_CHARS como elemento de la lista.
        MB_ERR_INVALID_CHARS,
        // Ejecuta value.c_str(),.
        value.c_str(),
        // Aporta -1 como elemento de la lista.
        -1,
        // Ejecuta result.data(),.
        result.data(),
        // Incorpora required en esta operación.
        required
    // Evalúa ) <= 0) { como parte de la condición.
    ) <= 0) {
        // Interrumpe la operación con std::system_error(.
        throw std::system_error(
            // Ejecuta static_cast<int>(GetLastError()),.
            static_cast<int>(GetLastError()),
            // Ejecuta std::system_category(),.
            std::system_category(),
            // Aporta el texto "MultiByteToWideChar(shared-memory name)" a la expresión.
            "MultiByteToWideChar(shared-memory name)"
        );
    }
    // Devuelve result.
    return result;
}

// Cierra la condición de compilación.
#endif

// Incorpora } // namespace en esta operación.
}  // namespace

// Declara el tipo SharedMemoryRegion y las operaciones o datos de su contrato.
struct SharedMemoryRegion::Impl final {
    // Declara name con el tipo indicado.
    std::string name;
    // Inicializa size con inicialización por valor.
    std::size_t size{};
    // Inicializa data con nullptr.
    std::byte* data{nullptr};
// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Inicializa mapping con nullptr.
    HANDLE mapping{nullptr};
// Abre la alternativa de compilación.
#else
    // Inicializa descriptor (descriptor del dispositivo) con -1.
    int descriptor{-1};
    // Declara native_name con el tipo indicado.
    std::string native_name;
// Cierra la condición de compilación.
#endif
};

// Ejecuta SharedMemoryRegion::SharedMemoryRegion(.
SharedMemoryRegion::SharedMemoryRegion(
    // Declara el parámetro name.
    std::string name,
    // Incorpora const std::size_t size en esta operación.
    const std::size_t size
)
    // Inicializa el miembro indicado por impl_(std::make_unique<Impl>()) { antes del cuerpo.
    : impl_(std::make_unique<Impl>()) {
    // Ejecuta validate_name(name).
    validate_name(name);
    // Evalúa la condición size == 0.
    if (size == 0) {
        // Interrumpe la operación con std::invalid_argument("Shared-memory size must be positive.").
        throw std::invalid_argument("Shared-memory size must be positive.");
    }

    // Establece impl_->name a partir de std::move(name).
    impl_->name = std::move(name);
    // Establece impl_->size a partir de size.
    impl_->size = size;

// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Establece wide_size a partir de static_cast<std::uint64_t>(size).
    const std::uint64_t wide_size = static_cast<std::uint64_t>(size);
    // Establece high a partir de static_cast<DWORD>(wide_size >> 32U).
    const DWORD high = static_cast<DWORD>(wide_size >> 32U);
    // Establece low a partir de static_cast<DWORD>(wide_size & 0xffffffffULL).
    const DWORD low = static_cast<DWORD>(wide_size & 0xffffffffULL);
    // Establece native_name a partir de utf8_to_wide(impl_->name).
    const std::wstring native_name = utf8_to_wide(impl_->name);

    // Establece impl_->mapping a partir de CreateFileMappingW(.
    impl_->mapping = CreateFileMappingW(
        // Aporta INVALID_HANDLE_VALUE como elemento de la lista.
        INVALID_HANDLE_VALUE,
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Aporta PAGE_READWRITE como elemento de la lista.
        PAGE_READWRITE,
        // Aporta high como elemento de la lista.
        high,
        // Aporta low como elemento de la lista.
        low,
        // Ejecuta native_name.c_str().
        native_name.c_str()
    );
    // Evalúa la condición impl_->mapping == nullptr.
    if (impl_->mapping == nullptr) {
        // Interrumpe la operación con std::system_error(.
        throw std::system_error(
            // Ejecuta static_cast<int>(GetLastError()),.
            static_cast<int>(GetLastError()),
            // Ejecuta std::system_category(),.
            std::system_category(),
            // Aporta el texto "CreateFileMappingW" a la expresión.
            "CreateFileMappingW"
        );
    }
    // Evalúa la condición GetLastError() == ERROR_ALREADY_EXISTS.
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Ejecuta CloseHandle(impl_->mapping).
        CloseHandle(impl_->mapping);
        // Establece impl_->mapping a partir de nullptr.
        impl_->mapping = nullptr;
        // Interrumpe la operación con std::runtime_error(.
        throw std::runtime_error(
            // Incorpora "A producer already owns shared memory '" + impl_->name + "'." en esta operación.
            "A producer already owns shared memory '" + impl_->name + "'."
        );
    }

    // Establece mapped a partir de MapViewOfFile(.
    void* mapped = MapViewOfFile(
        // Aporta impl_->mapping como elemento de la lista.
        impl_->mapping,
        // Aporta FILE_MAP_ALL_ACCESS como elemento de la lista.
        FILE_MAP_ALL_ACCESS,
        // Aporta 0 como elemento de la lista.
        0,
        // Aporta 0 como elemento de la lista.
        0,
        // Incorpora size en esta operación.
        size
    );
    // Evalúa la condición mapped == nullptr.
    if (mapped == nullptr) {
        // Establece error a partir de GetLastError().
        const DWORD error = GetLastError();
        // Ejecuta CloseHandle(impl_->mapping).
        CloseHandle(impl_->mapping);
        // Establece impl_->mapping a partir de nullptr.
        impl_->mapping = nullptr;
        // Interrumpe la operación con std::system_error(.
        throw std::system_error(
            // Aporta static_cast<int>(error) como elemento de la lista.
            static_cast<int>(error),
            // Ejecuta std::system_category(),.
            std::system_category(),
            // Aporta el texto "MapViewOfFile" a la expresión.
            "MapViewOfFile"
        );
    }
    // Establece impl_->data a partir de static_cast<std::byte*>(mapped).
    impl_->data = static_cast<std::byte*>(mapped);
// Abre la alternativa de compilación.
#else
    // Evalúa la condición size > static_cast<std::size_t>(.
    if (size > static_cast<std::size_t>(
        // Ejecuta std::numeric_limits<off_t>::max().
        std::numeric_limits<off_t>::max()
    )) {
        // Interrumpe la operación con std::overflow_error("Shared-memory size exceeds off_t.").
        throw std::overflow_error("Shared-memory size exceeds off_t.");
    }

    // Establece impl_->native_name a partir de "/" + impl_->name.
    impl_->native_name = "/" + impl_->name;
    // Establece impl_->descriptor a partir de ::shm_open(.
    impl_->descriptor = ::shm_open(
        // Ejecuta impl_->native_name.c_str(),.
        impl_->native_name.c_str(),
        // Aporta O_CREAT | O_EXCL | O_RDWR como elemento de la lista.
        O_CREAT | O_EXCL | O_RDWR,
        // Incorpora 0600 en esta operación.
        0600
    );
    // Evalúa la condición impl_->descriptor < 0.
    if (impl_->descriptor < 0) {
        // Interrumpe la operación con std::system_error(.
        throw std::system_error(
            // Aporta errno como elemento de la lista.
            errno,
            // Ejecuta std::generic_category(),.
            std::generic_category(),
            // Ejecuta "shm_open(" + impl_->native_name + ")".
            "shm_open(" + impl_->native_name + ")"
        );
    }
    // Evalúa la condición ::ftruncate(impl_->descriptor, static_cast<off_t>(size)) != 0.
    if (::ftruncate(impl_->descriptor, static_cast<off_t>(size)) != 0) {
        // Establece error a partir de errno.
        const int error = errno;
        // Cierra el recurso mediante close.
        ::close(impl_->descriptor);
        // Establece impl_->descriptor a partir de -1.
        impl_->descriptor = -1;
        // Ejecuta ::shm_unlink(impl_->native_name.c_str()).
        ::shm_unlink(impl_->native_name.c_str());
        // Interrumpe la operación con std::system_error(.
        throw std::system_error(
            // Aporta error como elemento de la lista.
            error,
            // Ejecuta std::generic_category(),.
            std::generic_category(),
            // Aporta el texto "ftruncate(shared memory)" a la expresión.
            "ftruncate(shared memory)"
        );
    }

    // Establece mapped a partir de ::mmap(.
    void* mapped = ::mmap(
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Aporta size como elemento de la lista.
        size,
        // Aporta PROT_READ | PROT_WRITE como elemento de la lista.
        PROT_READ | PROT_WRITE,
        // Aporta MAP_SHARED como elemento de la lista.
        MAP_SHARED,
        // Aporta impl_->descriptor como elemento de la lista.
        impl_->descriptor,
        // Incorpora 0 en esta operación.
        0
    );
    // Evalúa la condición mapped == MAP_FAILED.
    if (mapped == MAP_FAILED) {
        // Establece error a partir de errno.
        const int error = errno;
        // Cierra el recurso mediante close.
        ::close(impl_->descriptor);
        // Establece impl_->descriptor a partir de -1.
        impl_->descriptor = -1;
        // Ejecuta ::shm_unlink(impl_->native_name.c_str()).
        ::shm_unlink(impl_->native_name.c_str());
        // Interrumpe la operación con std::system_error(.
        throw std::system_error(
            // Aporta error como elemento de la lista.
            error,
            // Ejecuta std::generic_category(),.
            std::generic_category(),
            // Aporta el texto "mmap(shared memory)" a la expresión.
            "mmap(shared memory)"
        );
    }
    // Establece impl_->data a partir de static_cast<std::byte*>(mapped).
    impl_->data = static_cast<std::byte*>(mapped);
// Cierra la condición de compilación.
#endif
}

// Ejecuta SharedMemoryRegion::~SharedMemoryRegion() {.
SharedMemoryRegion::~SharedMemoryRegion() {
    // Evalúa la condición impl_ == nullptr.
    if (impl_ == nullptr) {
        // Termina la función sin un valor de retorno.
        return;
    }
// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Evalúa la condición impl_->data != nullptr.
    if (impl_->data != nullptr) {
        // Ejecuta UnmapViewOfFile(impl_->data).
        UnmapViewOfFile(impl_->data);
    }
    // Evalúa la condición impl_->mapping != nullptr.
    if (impl_->mapping != nullptr) {
        // Ejecuta CloseHandle(impl_->mapping).
        CloseHandle(impl_->mapping);
    }
// Abre la alternativa de compilación.
#else
    // Evalúa la condición impl_->data != nullptr.
    if (impl_->data != nullptr) {
        // Libera la región mapeada mediante munmap.
        ::munmap(impl_->data, impl_->size);
    }
    // Evalúa la condición impl_->descriptor >= 0.
    if (impl_->descriptor >= 0) {
        // Cierra el recurso mediante close.
        ::close(impl_->descriptor);
    }
    // Evalúa la condición !impl_->native_name.empty().
    if (!impl_->native_name.empty()) {
        // Ejecuta ::shm_unlink(impl_->native_name.c_str()).
        ::shm_unlink(impl_->native_name.c_str());
    }
// Cierra la condición de compilación.
#endif
}

// Define la operación SharedMemoryRegion::data con los parámetros declarados.
std::byte* SharedMemoryRegion::data() noexcept {
    // Devuelve impl_->data.
    return impl_->data;
}

// Define la operación SharedMemoryRegion::data con los parámetros declarados.
const std::byte* SharedMemoryRegion::data() const noexcept {
    // Devuelve impl_->data.
    return impl_->data;
}

// Define la operación SharedMemoryRegion::size con los parámetros declarados.
std::size_t SharedMemoryRegion::size() const noexcept {
    // Devuelve impl_->size.
    return impl_->size;
}

// Define la operación SharedMemoryRegion::name con los parámetros declarados.
const std::string& SharedMemoryRegion::name() const noexcept {
    // Devuelve impl_->name.
    return impl_->name;
}

// Incorpora } // namespace orvix::ipc en esta operación.
}  // namespace orvix::ipc
