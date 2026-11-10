// Aplica la directiva del compilador once.
#pragma once

// Incluye filesystem para usar rutas y operaciones de archivos.
#include <filesystem>
// Incluye fstream para usar flujos para archivos.
#include <fstream>
// Incluye mutex para usar exclusión mutua.
#include <mutex>
// Incluye string_view para usar vistas de cadenas.
#include <string_view>

// Agrupa las declaraciones bajo orvix::observability.
namespace orvix::observability {

// Declara el tipo LogLevel y las operaciones o datos de su contrato.
enum class LogLevel {
    // Aporta info como elemento de la lista.
    info,
    // Aporta warning como elemento de la lista.
    warning,
    // Incorpora error en esta operación.
    error
};

// Declara el tipo Logger y las operaciones o datos de su contrato.
class Logger final {
// Expone los miembros de la interfaz.
public:
    // Define la operación r con los parámetros declarados.
    explicit Logger(
        // Establece path a partir de default_log_path().
        std::filesystem::path path = default_log_path()
    );

    // Prohíbe esta operación en tiempo de compilación.
    Logger(const Logger&) = delete;
    // Establece operator a partir de (const Logger&) = delete.
    Logger& operator=(const Logger&) = delete;
    // Prohíbe esta operación en tiempo de compilación.
    Logger(Logger&&) = delete;
    // Establece operator a partir de (Logger&&) = delete.
    Logger& operator=(Logger&&) = delete;

    // Define la operación write, que escribe en el recurso.
    void write(
        // Declara el parámetro level.
        LogLevel level,
        // Declara el parámetro code.
        std::string_view code,
        // Declara el parámetro event.
        std::string_view event,
        // Incorpora std::string_view message en esta operación.
        std::string_view message
    );

    // Define la operación info con los parámetros declarados.
    void info(
        // Declara el parámetro code.
        std::string_view code,
        // Declara el parámetro event.
        std::string_view event,
        // Incorpora std::string_view message en esta operación.
        std::string_view message
    );

    // Define la operación warning con los parámetros declarados.
    void warning(
        // Declara el parámetro code.
        std::string_view code,
        // Declara el parámetro event.
        std::string_view event,
        // Incorpora std::string_view message en esta operación.
        std::string_view message
    );

    // Define la operación error con los parámetros declarados.
    void error(
        // Declara el parámetro code.
        std::string_view code,
        // Declara el parámetro event.
        std::string_view event,
        // Incorpora std::string_view message en esta operación.
        std::string_view message
    );

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta const std::filesystem::path& path() const noexcept.
    const std::filesystem::path& path() const noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta static std::filesystem::path default_log_path().
    static std::filesystem::path default_log_path();

// Restringe estos miembros a la implementación.
private:
    // Declara path_ con el tipo indicado.
    std::filesystem::path path_;
    // Declara stream_ con el tipo indicado.
    std::ofstream stream_;
    // Declara mutex_ (mutex que protege el acceso compartido) con el tipo indicado.
    std::mutex mutex_;
};

// Incorpora } // namespace orvix::observability en esta operación.
}  // namespace orvix::observability
