#include "lsp/ProjectContext.h"
#include "namespace/ProjectScanner.h"
#include "ffi/CHeaderResolver.h"

#include <filesystem>

namespace fs = std::filesystem;

// ═══════════════════════════════════════════════════════════════════════
//  Build project context
// ═══════════════════════════════════════════════════════════════════════

bool ProjectContext::build(const std::string& filePath) {
    valid_ = false;
    units_.clear();
    fileNamespaces_.clear();
    registry_ = NamespaceRegistry();
    cBindings_ = CBindings();
    cTypeReg_ = TypeRegistry();

    projectRoot_ = findProjectRoot(filePath);
    if (projectRoot_.empty()) return false;

    // Scan all .lx files in the project.
    auto allFiles = ProjectScanner::scan(projectRoot_);
    if (allFiles.empty()) return false;

    // Parse all files and build the registry.
    for (auto& path : allFiles) {
        SourceUnit unit;
        unit.filePath    = path;
        unit.parseResult = Parser::parse(path);

        if (unit.parseResult.hasErrors || !unit.parseResult.tree)
            continue;

        unit.namespaceName = extractNamespace(unit.parseResult.tree);
        if (unit.namespaceName.empty())
            continue;

        registry_.registerFile(unit.namespaceName, path,
                               unit.parseResult.tree);
        fileNamespaces_[path] = unit.namespaceName;

        // Resolve C headers from this file.
        std::vector<LuxParser::IncludeDeclContext*> includes;
        for (auto* pre : unit.parseResult.tree->preambleDecl())
            if (auto* inc = pre->includeDecl()) includes.push_back(inc);
        if (!includes.empty()) {
            CHeaderResolver resolver(cTypeReg_, cBindings_);
            for (auto* incl : includes) {
                auto text = incl->getText();
                if (incl->INCLUDE_SYS()) {
                    auto header = CHeaderResolver::extractSystemHeader(text);
                    if (!header.empty())
                        resolver.resolveSystemHeader(header);
                } else if (incl->INCLUDE_LOCAL()) {
                    auto header = CHeaderResolver::extractLocalHeader(text);
                    if (!header.empty()) {
                        auto dir = fs::path(path).parent_path().string();
                        resolver.resolveLocalHeader(header, dir);
                    }
                }
            }
        }

        units_.push_back(std::move(unit));
    }

    valid_ = true;
    return true;
}

std::string ProjectContext::namespaceFor(const std::string& filePath) const {
    // Try exact match first.
    auto it = fileNamespaces_.find(filePath);
    if (it != fileNamespaces_.end()) return it->second;

    // Try canonical path.
    try {
        auto canon = fs::canonical(filePath).string();
        it = fileNamespaces_.find(canon);
        if (it != fileNamespaces_.end()) return it->second;
    } catch (...) {}

    return "";
}

// ═══════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════

std::string ProjectContext::findProjectRoot(const std::string& filePath) {
    // Walk up from the file's directory looking for the project root.
    // Heuristic: the root is the highest directory that still contains .lx files.
    try {
        auto dir = fs::canonical(fs::path(filePath).parent_path());
        std::string bestRoot = dir.string();

        while (dir.has_parent_path() && dir != dir.parent_path()) {
            auto parent = dir.parent_path();
            // Check if parent still has .lx files (directly or in subdirs)
            bool hasLx = false;
            for (auto& entry : fs::directory_iterator(parent)) {
                if (entry.is_regular_file() && entry.path().extension() == ".lx") {
                    hasLx = true;
                    break;
                }
                if (entry.is_directory()) {
                    auto dirName = entry.path().filename().string();
                    if (!dirName.empty() && dirName[0] == '.') continue;
                    // Quick check: does this subdir have any .lx?
                    for (auto& sub : fs::recursive_directory_iterator(
                             entry.path(), fs::directory_options::skip_permission_denied)) {
                        if (sub.is_regular_file() && sub.path().extension() == ".lx") {
                            hasLx = true;
                            break;
                        }
                    }
                    if (hasLx) break;
                }
            }
            if (hasLx) {
                bestRoot = parent.string();
                dir = parent;
            } else {
                break;
            }
        }

        return bestRoot;
    } catch (...) {
        // If path resolution fails, use the file's directory.
        return fs::path(filePath).parent_path().string();
    }
}

std::string ProjectContext::extractNamespace(LuxParser::ProgramContext* tree) {
    if (!tree) return "";
    auto* nsDecl = tree->namespaceDecl();
    if (!nsDecl) return "";
    return nsDecl->IDENTIFIER()->getText();
}
