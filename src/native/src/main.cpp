// Incluye las declaraciones de capture cli de ORVIX.
#include "orvix/cli/capture_cli.hpp"

// Define la operación main con los parámetros declarados.
int main(const int argc, char* argv[]) {
    // Crea el objeto que interpreta las órdenes de captura.
    const orvix::cli::CaptureCli cli;
    // Entrega al CLI los argumentos del proceso y devuelve su código de salida.
    return cli.run(argc, argv);
}
