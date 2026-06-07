#include "cli/CheckCommand.h"
#include "cli/ArgParser.h"
#include "cli/LuxPipeline.h"

#include <iostream>

void CheckCommand::buildArgs(ArgParser& parser) const {
    parser.addPositional("file", "Path to the .lx entrypoint file");
    parser.addFlag("quiet", 'q', "Suppress pipeline logs");
    parser.addOption("include", 'I', "DIR", "Add include search path (repeatable)", true);
}

int CheckCommand::run(const ArgParser& parser) {
    LuxPipeline::Options pipeOpts;
    pipeOpts.inputFile    = parser.get("file");
    pipeOpts.quiet        = parser.has("quiet");
    pipeOpts.includePaths = parser.getAll("include");

    auto pipeline = LuxPipeline::run(pipeOpts);
    if (!pipeline || pipeline->hasErrors) return 1;

    if (!pipeOpts.quiet)
        std::cerr << "lux: [check] no errors found\n";
    return 0;
}
