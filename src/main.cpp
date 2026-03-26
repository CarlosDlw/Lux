#include "cli/CLI.h"

int main(int argc, char* argv[]) {
    CLI cli(argc, argv);
    return cli.run();
}
