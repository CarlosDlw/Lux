#include "lsp/LspServer.h"

#include <iostream>

// LSP error codes
static constexpr int kParseError     = -32700;
static constexpr int kMethodNotFound = -32601;
static constexpr int kInvalidRequest = -32600;

// ═══════════════════════════════════════════════════════════════════════
//  Main loop
// ═══════════════════════════════════════════════════════════════════════

int LspServer::run() {
    // LSP uses stderr for logging (stdout is the protocol channel)
    std::cerr << "[lux-lsp] server started\n";

    while (!shutdown_) {
        auto raw = transport_.readMessage();
        if (!raw) {
            break; // EOF or read error
        }

        json msg;
        try {
            msg = json::parse(*raw);
        } catch (const json::parse_error& e) {
            std::cerr << "[lux-lsp] JSON parse error: " << e.what() << "\n";
            if (msg.contains("id")) {
                sendError(msg["id"], kParseError, "JSON parse error");
            }
            continue;
        }

        dispatch(msg);
    }

    std::cerr << "[lux-lsp] server stopped\n";
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════
//  Dispatch
// ═══════════════════════════════════════════════════════════════════════

void LspServer::dispatch(const json& msg) {
    if (!msg.contains("method")) {
        // Response or invalid — ignore for now
        return;
    }

    std::string method = msg["method"];

    if (method == "initialize") {
        handleInitialize(msg);
    } else if (method == "initialized") {
        handleInitialized(msg);
    } else if (method == "shutdown") {
        handleShutdown(msg);
    } else if (method == "exit") {
        shutdown_ = true;
    } else if (method == "textDocument/didOpen") {
        handleDidOpen(msg);
    } else if (method == "textDocument/didChange") {
        handleDidChange(msg);
    } else if (method == "textDocument/didSave") {
        handleDidSave(msg);
    } else if (method == "textDocument/didClose") {
        handleDidClose(msg);
    } else {
        // Unknown method — if it has an id, it's a request → respond with error
        if (msg.contains("id")) {
            sendError(msg["id"], kMethodNotFound,
                      "method not supported: " + method);
        }
        // Notifications without id are silently ignored
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  LSP Lifecycle
// ═══════════════════════════════════════════════════════════════════════

void LspServer::handleInitialize(const json& msg) {
    std::cerr << "[lux-lsp] initialize\n";

    json capabilities = {
        {"textDocumentSync", {
            {"openClose", true},
            {"save", {{"includeText", true}}},
            {"change", 1}  // Full document sync
        }}
    };

    json result = {
        {"capabilities", capabilities},
        {"serverInfo", {
            {"name", "lux-lsp"},
            {"version", "0.1.0"}
        }}
    };

    sendResponse(msg["id"], result);
    initialized_ = true;
}

void LspServer::handleInitialized(const json& /*msg*/) {
    std::cerr << "[lux-lsp] initialized\n";
}

void LspServer::handleShutdown(const json& msg) {
    std::cerr << "[lux-lsp] shutdown\n";
    sendResponse(msg["id"], nullptr);
    shutdown_ = true;
}

// ═══════════════════════════════════════════════════════════════════════
//  Document Sync
// ═══════════════════════════════════════════════════════════════════════

void LspServer::handleDidOpen(const json& msg) {
    auto& params = msg["params"];
    auto& td     = params["textDocument"];
    std::string uri  = td["uri"];
    std::string text = td["text"];

    documents_.open(uri, text);
    std::cerr << "[lux-lsp] opened: " << uri << "\n";

    publishDiagnostics(uri, text);
}

void LspServer::handleDidSave(const json& msg) {
    auto& params = msg["params"];
    auto& td     = params["textDocument"];
    std::string uri = td["uri"];

    std::string text;
    if (params.contains("text")) {
        text = params["text"];
        documents_.update(uri, text);
    } else {
        text = documents_.get(uri);
    }

    std::cerr << "[lux-lsp] saved: " << uri << "\n";

    publishDiagnostics(uri, text);
}

void LspServer::handleDidChange(const json& msg) {
    auto& params = msg["params"];
    std::string uri = params["textDocument"]["uri"];

    // Full document sync (change mode 1): contentChanges[0].text is the full text
    auto& changes = params["contentChanges"];
    if (changes.empty()) return;

    std::string text = changes[0]["text"];
    documents_.update(uri, text);

    publishDiagnostics(uri, text);
}

void LspServer::handleDidClose(const json& msg) {
    auto& params = msg["params"];
    std::string uri = params["textDocument"]["uri"];

    documents_.close(uri);
    std::cerr << "[lux-lsp] closed: " << uri << "\n";

    // Clear diagnostics for the closed file
    sendNotification("textDocument/publishDiagnostics", {
        {"uri", uri},
        {"diagnostics", json::array()}
    });
}

// ═══════════════════════════════════════════════════════════════════════
//  Diagnostics
// ═══════════════════════════════════════════════════════════════════════

void LspServer::publishDiagnostics(const std::string& uri,
                                    const std::string& source) {
    auto diags = diagnosticEngine_.run(source);

    json lspDiags = json::array();
    for (auto& d : diags) {
        lspDiags.push_back({
            {"range", {
                {"start", {{"line", d.line}, {"character", d.col}}},
                {"end",   {{"line", d.endLine}, {"character", d.endCol}}}
            }},
            {"severity", static_cast<int>(d.severity)},
            {"source", d.source},
            {"message", d.message}
        });
    }

    sendNotification("textDocument/publishDiagnostics", {
        {"uri", uri},
        {"diagnostics", lspDiags}
    });

    std::cerr << "[lux-lsp] published " << diags.size()
              << " diagnostic(s) for " << uri << "\n";
}

// ═══════════════════════════════════════════════════════════════════════
//  JSON-RPC helpers
// ═══════════════════════════════════════════════════════════════════════

void LspServer::sendResponse(const json& id, const json& result) {
    json response = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", result}
    };
    transport_.sendMessage(response.dump());
}

void LspServer::sendError(const json& id, int code,
                           const std::string& message) {
    json response = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    };
    transport_.sendMessage(response.dump());
}

void LspServer::sendNotification(const std::string& method,
                                  const json& params) {
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params}
    };
    transport_.sendMessage(notification.dump());
}
