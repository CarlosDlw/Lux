#pragma once

#include "lsp/LspTransport.h"
#include "lsp/DocumentStore.h"
#include "lsp/DiagnosticEngine.h"

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class LspServer {
public:
    // Main loop — blocks until shutdown or EOF.
    int run();

private:
    LspTransport    transport_;
    DocumentStore   documents_;
    DiagnosticEngine diagnosticEngine_;
    bool            initialized_ = false;
    bool            shutdown_    = false;

    // Dispatch a parsed JSON-RPC message.
    void dispatch(const json& msg);

    // LSP lifecycle
    void handleInitialize(const json& msg);
    void handleInitialized(const json& msg);
    void handleShutdown(const json& msg);

    // Document sync
    void handleDidOpen(const json& msg);
    void handleDidChange(const json& msg);
    void handleDidSave(const json& msg);
    void handleDidClose(const json& msg);

    // Publish diagnostics for a document.
    void publishDiagnostics(const std::string& uri, const std::string& source);

    // Helpers
    void sendResponse(const json& id, const json& result);
    void sendError(const json& id, int code, const std::string& message);
    void sendNotification(const std::string& method, const json& params);
};
