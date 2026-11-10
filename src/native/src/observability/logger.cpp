// Incluye las declaraciones de logger de ORVIX.
#include "orvix/observability/logger.hpp"

// Incluye chrono para usar relojes y duraciones.
#include <chrono>
// Importa las declaraciones de ctime para esta unidad.
#include <ctime>
// Incluye iomanip para usar formato de flujos.
#include <iomanip>
// Incluye sstream para usar flujos de texto en memoria.
#include <sstream>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye string para usar cadenas de texto.
#include <string>
// Importa las declaraciones de system_error para esta unidad.
#include <system_error>
// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>

// Agrupa las declaraciones bajo orvix::observability.
namespace orvix::observability {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Define la operación level_name con los parámetros declarados.
std::string_view level_name(const LogLevel level) noexcept {
    // Selecciona una rama según level.
    switch (level) {
    // Selecciona el caso LogLevel::info.
    case LogLevel::info:
        // Devuelve "INFO".
        return "INFO";
    // Selecciona el caso LogLevel::warning.
    case LogLevel::warning:
        // Devuelve "WARNING".
        return "WARNING";
    // Selecciona el caso LogLevel::error.
    case LogLevel::error:
        // Devuelve "ERROR".
        return "ERROR";
    }

    // Devuelve "UNKNOWN".
    return "UNKNOWN";
}

// Define la operación utc_timestamp con los parámetros declarados.
std::string utc_timestamp() {
    // Establece now a partir de std::chrono::system_clock::now().
    const auto now = std::chrono::system_clock::now();
    // Establece value a partir de std::chrono::system_clock::to_time_t(now).
    const std::time_t value = std::chrono::system_clock::to_time_t(now);
    // Inicializa utc con inicialización por valor.
    std::tm utc{};

// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Establece conversion_failed a partir de gmtime_s(&utc, &value) != 0.
    const bool conversion_failed = gmtime_s(&utc, &value) != 0;
// Abre la alternativa de compilación.
#else
    // Establece conversion_failed a partir de gmtime_r(&value, &utc) == nullptr.
    const bool conversion_failed = gmtime_r(&value, &utc) == nullptr;
// Cierra la condición de compilación.
#endif

    // Evalúa la condición conversion_failed.
    if (conversion_failed) {
        // Interrumpe la operación con std::runtime_error("Unable to create a UTC log timestamp.").
        throw std::runtime_error("Unable to create a UTC log timestamp.");
    }

    // Declara stream con el tipo indicado.
    std::ostringstream stream;
    // Ejecuta stream << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ").
    stream << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    // Devuelve stream.str().
    return stream.str();
}

// Define la operación escape_message con los parámetros declarados.
std::string escape_message(const std::string_view message) {
    // Declara escaped con el tipo indicado.
    std::string escaped;
    // Reserva capacidad en el contenedor mediante escaped.reserve.
    escaped.reserve(message.size());

    // Recorre según const char value : message.
    for (const char value : message) {
        // Selecciona una rama según value.
        switch (value) {
        // Selecciona el caso '\\'.
        case '\\':
            // Actualiza escaped con += y "\\\\".
            escaped += "\\\\";
            // Termina el bucle o la rama actual.
            break;
        // Selecciona el caso '"'.
        case '"':
            // Actualiza escaped con += y "\\\"".
            escaped += "\\\"";
            // Termina el bucle o la rama actual.
            break;
        // Selecciona el caso '\r'.
        case '\r':
        // Selecciona el caso '\n'.
        case '\n':
            // Actualiza escaped con += y ' '.
            escaped += ' ';
            // Termina el bucle o la rama actual.
            break;
        // Atiende los valores que no coinciden con los casos anteriores.
        default:
            // Actualiza escaped con += y value.
            escaped += value;
            // Termina el bucle o la rama actual.
            break;
        }
    }

    // Devuelve escaped.
    return escaped;
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Ejecuta Logger::Logger(std::filesystem::path path).
Logger::Logger(std::filesystem::path path)
    // Inicializa el miembro indicado por path_(std::move(path)) { antes del cuerpo.
    : path_(std::move(path)) {
    // Establece parent a partir de path_.parent_path().
    const std::filesystem::path parent = path_.parent_path();

    // Evalúa la condición !parent.empty().
    if (!parent.empty()) {
        // Declara error con el tipo indicado.
        std::error_code error;
        // Ejecuta std::filesystem::create_directories(parent, error).
        std::filesystem::create_directories(parent, error);

        // Evalúa la condición error.
        if (error) {
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Ejecuta "Unable to create log directory: " + error.message().
                "Unable to create log directory: " + error.message()
            );
        }
    }

    // Ejecuta stream_.open(path_, std::ios::app).
    stream_.open(path_, std::ios::app);

    // Evalúa la condición !stream_.is_open().
    if (!stream_.is_open()) {
        // Interrumpe la operación con std::runtime_error(.
        throw std::runtime_error(
            // Ejecuta "Unable to open capture log: " + path_.string().
            "Unable to open capture log: " + path_.string()
        );
    }
}

// Define la operación Logger::write con los parámetros declarados.
void Logger::write(
    // Declara el parámetro level.
    const LogLevel level,
    // Declara el parámetro code.
    const std::string_view code,
    // Declara el parámetro event.
    const std::string_view event,
    // Incorpora const std::string_view message en esta operación.
    const std::string_view message
) {
    // Ejecuta const std::lock_guard lock(mutex_).
    const std::lock_guard lock(mutex_);

    // Incorpora stream_ en esta operación.
    stream_
        // Establece timestamp (marca de tiempo) a partir de " << utc_timestamp().
        << "timestamp=" << utc_timestamp()
        // Establece level a partir de " << level_name(level).
        << " level=" << level_name(level)
        // Establece code a partir de " << code.
        << " code=" << code
        // Establece event a partir de " << event.
        << " event=" << event
        // Establece message a partir de \"" << escape_message(message) << "\"\n".
        << " message=\"" << escape_message(message) << "\"\n";
    // Ejecuta stream_.flush().
    stream_.flush();

    // Evalúa la condición !stream_.
    if (!stream_) {
        // Interrumpe la operación con std::runtime_error(.
        throw std::runtime_error(
            // Ejecuta "Unable to write capture log: " + path_.string().
            "Unable to write capture log: " + path_.string()
        );
    }
}

// Define la operación Logger::info con los parámetros declarados.
void Logger::info(
    // Declara el parámetro code.
    const std::string_view code,
    // Declara el parámetro event.
    const std::string_view event,
    // Incorpora const std::string_view message en esta operación.
    const std::string_view message
) {
    // Escribe en el recurso mediante write.
    write(LogLevel::info, code, event, message);
}

// Define la operación Logger::warning con los parámetros declarados.
void Logger::warning(
    // Declara el parámetro code.
    const std::string_view code,
    // Declara el parámetro event.
    const std::string_view event,
    // Incorpora const std::string_view message en esta operación.
    const std::string_view message
) {
    // Escribe en el recurso mediante write.
    write(LogLevel::warning, code, event, message);
}

// Define la operación Logger::error con los parámetros declarados.
void Logger::error(
    // Declara el parámetro code.
    const std::string_view code,
    // Declara el parámetro event.
    const std::string_view event,
    // Incorpora const std::string_view message en esta operación.
    const std::string_view message
) {
    // Escribe en el recurso mediante write.
    write(LogLevel::error, code, event, message);
}

// Define la operación Logger::path con los parámetros declarados.
const std::filesystem::path& Logger::path() const noexcept {
    // Devuelve path_.
    return path_;
}

// Define la operación Logger::default_log_path con los parámetros declarados.
std::filesystem::path Logger::default_log_path() {
    // Devuelve std::filesystem::path{"logs"} / "orvix-capture.log".
    return std::filesystem::path{"logs"} / "orvix-capture.log";
}

// Incorpora } // namespace orvix::observability en esta operación.
}  // namespace orvix::observability
