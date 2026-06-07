#include "cli/HelpCommand.h"
#include "cli/ArgParser.h"

#include <iostream>
#include <string>

void HelpCommand::buildArgs(ArgParser& parser) const {
    parser.addPositional("command", "Command to get help for (optional)", false);
}

int HelpCommand::run(const ArgParser& parser) {
    std::string cmd = parser.get("command");
    if (cmd.empty()) {
        printGeneralHelp();
        return 0;
    }

    // Show help for a specific command
    for (auto* c : commands_) {
        if (c->name() == cmd) {
            ArgParser cmdParser("lux " + c->name(), c->description());
            c->buildArgs(cmdParser);
            cmdParser.printHelp();
            return 0;
        }
    }

    std::cerr << "lux: unknown command '" << cmd << "'\n";
    std::cerr << "Run 'lux help' for usage.\n";
    return 1;
}

void HelpCommand::printGeneralHelp() const {
    std::cout << "lux — The Lux compiler\n\n"
              << "Usage:\n"
              << "  lux <command> [args...]\n\n"
              << "Commands:\n";

    for (auto* c : commands_) {
        if (c->name() == "help" || c->name() == "version") continue;
        std::cout << "  " << c->name();
        size_t pad = c->name().size() < 12 ? 12 - c->name().size() : 1;
        std::cout << std::string(pad, ' ') << c->description() << "\n";
    }

    std::cout << "\nExamples:\n"
              << "  lux build main.lx -o ./main -O2\n"
              << "  lux build main.lx --emit-llvm\n"
              << "  lux run main.lx\n"
              << "  lux check main.lx\n"
              << "  lux test\n"
              << "  lux help build\n"
              << "  lux helpc raylib InitWindow\n";
}
