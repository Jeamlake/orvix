#include "orvix/cli/capture_cli.hpp"

int main(const int argc, char* argv[]) {
    const orvix::cli::CaptureCli cli;
    return cli.run(argc, argv);
}
